#include "websocketimageclient.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <future>
#include <QDebug>
#include <memory>
#include <iostream>
#include <QTimer>
#include <QMetaObject>
#include "control.pb.h"

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;
using imagesocket::control::ControlMessage;

struct WebSocketImageClient::Impl {
    Impl() = default;
    ~Impl() = default;

    std::unique_ptr<asio::io_context> ioc;
    std::unique_ptr<std::thread> ioThread;
    std::shared_ptr<websocket::stream<tcp::socket>> ws;
    std::atomic<bool> running{false};
    std::atomic<bool> reconnecting{false};
    std::mutex mtx;
    std::condition_variable cv;

    // Configured FPS persisted from server SET_FPS messages (0 == unset)
    std::atomic<int> configuredFps{0};
};

WebSocketImageClient::WebSocketImageClient(const QString &host, quint16 port, QObject* parent)
    : QObject(parent), m_host(host), m_port(port)
{
    m_impl = new Impl();
}

WebSocketImageClient::~WebSocketImageClient()
{
    disconnectFromServer();
    delete m_impl;
}

bool WebSocketImageClient::connectToServer()
{
    if (m_impl->running.load())
        return true;

    // Cleanup previous connection
    cleanupConnection();

    m_impl->running.store(true);
    m_impl->ioc.reset(new asio::io_context());

    try {
        tcp::resolver resolver(*m_impl->ioc);
        auto const results = resolver.resolve(m_host.toStdString(), std::to_string(m_port));

        tcp::socket socket(*m_impl->ioc);
        asio::connect(socket, results);

        // Keep io_context alive while async ops are pending
        auto workGuard = asio::make_work_guard(*m_impl->ioc);
        auto guardPtr = std::make_shared<decltype(workGuard)>(std::move(workGuard));

        m_impl->ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
        
        // Configure WebSocket options for large frames
        m_impl->ws->binary(true);
        m_impl->ws->auto_fragment(true);
        m_impl->ws->write_buffer_bytes(256 * 1024); // 256KB write buffer

        // Start io_context in background thread FIRST
        qInfo() << "Starting IO thread (id will be set after thread runs)";
        m_impl->ioThread.reset(new std::thread([this, guardPtr]() {
            Q_UNUSED(guardPtr); // Keep work guard alive during run()
            qInfo() << "IO thread started";
            try {
                m_impl->ioc->run();
            } catch (const std::exception &ex) {
                qWarning() << "IO thread exception:" << ex.what();
            }
            qInfo() << "IO thread exiting";

            // When the io_context run loop exits, request final cleanup from a non-IO thread.
            // Use a short detached helper thread to call cleanupConnection() so final reset happens
            // outside the io thread context. This avoids running cleanup from the IO thread.
            std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                cleanupConnection();
            }).detach();
        }));

        // Async handshake
        std::promise<void> hsPromise;
        auto hsFuture = hsPromise.get_future();

        m_impl->ws->async_handshake(m_host.toStdString() + ":" + std::to_string(m_port), "/",
            [&hsPromise](beast::error_code ec) {
                if (ec) {
                    std::cerr << "WebSocket handshake error: " << ec.message() << std::endl;
                    try {
                        throw beast::system_error(ec);
                    } catch(...) {
                        hsPromise.set_exception(std::current_exception());
                    }
                } else {
                    std::cout << "WebSocket handshake succeeded" << std::endl;
                    hsPromise.set_value();
                }
            }
        );

        // Wait for handshake to complete (timeout support)
        auto status = hsFuture.wait_for(std::chrono::seconds(5));
        if (status == std::future_status::timeout) {
            qWarning() << "WebSocket handshake timeout";
            cleanupConnection();
            return false;
        }

        try {
            hsFuture.get();
        } catch (const std::exception& ex) {
            qWarning() << "WebSocket handshake failed:" << ex.what();
            cleanupConnection();
            return false;
        }

        qInfo() << "Connected to WebSocket server";
        if (m_onConnected) m_onConnected();

        // Start async read loop for control messages
        doAsyncRead();

        return true;
    } catch (const std::exception &ex) {
        qWarning() << "WebSocketImageClient exception:" << ex.what();
        m_impl->running.store(false);
        return false;
    }
}

int WebSocketImageClient::configuredFps() const {
    return m_impl ? m_impl->configuredFps.load() : 0;
}

void WebSocketImageClient::setAlias(const QString& alias)
{
    m_alias = alias;
}

QString WebSocketImageClient::alias() const
{
    return m_alias;
}

void WebSocketImageClient::doAsyncRead()
{
    if (!m_impl->running.load() || !m_impl->ws) return;

    auto buffer = std::make_shared<beast::flat_buffer>();

    m_impl->ws->async_read(*buffer,
        [this, buffer](beast::error_code ec, std::size_t bytes_transferred) {
            (void)bytes_transferred;
            if (ec) {
                qWarning() << "WebSocket read error:" << QString::fromStdString(ec.message());
                cleanupConnection();
                return;
            }

            // Convert buffer to string to inspect prefix
            std::string s = beast::buffers_to_string(buffer->data());
            if (!s.empty()) {
                unsigned char prefix = static_cast<unsigned char>(s[0]);
                if (prefix == 0x01) {
                    // Control message
                    ControlMessage msg;
                    if (msg.ParseFromArray(s.data() + 1, static_cast<int>(s.size() - 1))) {
                        qInfo() << "Received ControlMessage type=" << msg.type();
                        if (msg.type() == imagesocket::control::REQUEST_ALIAS) {
                            // Reply with our alias (if any)
                            ControlMessage reply;
                            reply.set_type(imagesocket::control::ALIAS);
                            reply.set_alias(m_alias.toStdString());
                            std::string out;
                            if (reply.SerializeToString(&out)) {
                                QByteArray ba(out.data(), static_cast<int>(out.size()));
                                sendControlMessage(ba);
                            }
                        } else if (msg.type() == imagesocket::control::SET_FPS) {
                            int fps = msg.fps();
                            qInfo() << "Received SET_FPS from server:" << fps;
                            if (m_impl) {
                                m_impl->configuredFps.store(fps);
                            }
                            // invoke callback if set (note: may run on IO thread)
                            if (m_onFpsChanged) {
                                try { m_onFpsChanged(fps); } catch(...) {}
                            }
                        }
                    } else {
                        qWarning() << "Failed to parse ControlMessage from server";
                    }
                } else {
                    // Non-control message: ignore for now
                }
            }

            buffer->consume(buffer->size());
            // Schedule next read
            doAsyncRead();
        }
    );
}

void WebSocketImageClient::disconnectFromServer()
{
    std::cout << "WebSocketImageClient::disconnectFromServer called" << std::endl;
    cleanupConnection();
}

void WebSocketImageClient::cleanupConnection()
{
    // Ensure mutual exclusion for cleanup
    std::lock_guard<std::mutex> lock(m_impl->mtx);

    // Mark not running first to prevent new sends
    m_impl->running.store(false);

    // Close websocket if open
    try {
        if (m_impl->ws) {
            beast::error_code ec;
            if (m_impl->ws->is_open()) {
                m_impl->ws->close(websocket::close_code::normal, ec);
                if (ec)
                    qWarning() << "cleanup: ws close error" << QString::fromStdString(ec.message());
            }
        }
    } catch (...) {}

    // Stop the io_context
    if (m_impl->ioc) {
        m_impl->ioc->stop();
    }

    // Determine if we're running inside the io thread
    bool inIoThread = (m_impl->ioThread && std::this_thread::get_id() == m_impl->ioThread->get_id());

    // Join and clear thread (but don't join from the io thread itself)
    if (m_impl->ioThread && m_impl->ioThread->joinable()) {
        if (!inIoThread) {
            m_impl->ioThread->join();
            m_impl->ioThread.reset();
        } else {
            qInfo() << "cleanupConnection called from IO thread; deferring final resource reset until io thread exits";
            // do not reset ws/ioc here; the io thread will exit and control will return to a non-IO thread where cleanup can finish
        }
    }

    // If we're in the IO thread, avoid resetting shared resources here (prevents race / double-free)
    if (!inIoThread) {
        // Reset resources
        m_impl->ws.reset();
        m_impl->ioc.reset();
    } else {
        // Notify disconnected and start reconnect loop, but leave final resets until outside thread
        if (m_onDisconnected) {
            QMetaObject::invokeMethod(this, [this]() {
                if (m_onDisconnected) m_onDisconnected();
            }, Qt::QueuedConnection);
        }

        // Start an automatic reconnect loop (single instance)
        bool expected = false;
        if (!m_impl->reconnecting.load() && m_impl->reconnecting.compare_exchange_strong(expected, true)) {
            std::thread([this]() {
                int attempt = 0;
                while (!m_impl->running.load()) {
                    int waitSeconds = std::min(30, 1 << std::min(attempt, 6));
                    qInfo() << "Reconnect: attempt" << attempt << "waiting" << waitSeconds << "s";
                    std::this_thread::sleep_for(std::chrono::seconds(waitSeconds));
                    if (connectToServer()) {
                        qInfo() << "Reconnect succeeded";
                        m_impl->reconnecting.store(false);
                        return;
                    }
                    attempt++;
                }
                m_impl->reconnecting.store(false);
            }).detach();
        }

        // Defer remainder of cleanup; another call to cleanupConnection() from non-IO thread will handle it
        return;
    }

    // Notify disconnected on main thread
    if (m_onDisconnected) {
        QMetaObject::invokeMethod(this, [this]() {
            if (m_onDisconnected) m_onDisconnected();
        }, Qt::QueuedConnection);
    }

    // Start an automatic reconnect loop (single instance)
    bool expected = false;
    if (!m_impl->reconnecting.load() && m_impl->reconnecting.compare_exchange_strong(expected, true)) {
        std::thread([this]() {
            int attempt = 0;
            while (!m_impl->running.load()) {
                int waitSeconds = std::min(30, 1 << std::min(attempt, 6));
                qInfo() << "Reconnect: attempt" << attempt << "waiting" << waitSeconds << "s";
                std::this_thread::sleep_for(std::chrono::seconds(waitSeconds));
                if (connectToServer()) {
                    qInfo() << "Reconnect succeeded";
                    m_impl->reconnecting.store(false);
                    return;
                }
                attempt++;
            }
            m_impl->reconnecting.store(false);
        }).detach();
    }
}

bool WebSocketImageClient::sendFrame(const QByteArray &jpegData)
{
    if (!m_impl->running.load() || !m_impl->ws || !m_impl->ws->is_open()) {
        std::cerr << "sendFrame: not connected" << std::endl;
        m_impl->running.store(false);
        return false;}

    try {
        auto payload = std::make_shared<std::vector<unsigned char>>(
            reinterpret_cast<const unsigned char*>(jpegData.constData()),
            reinterpret_cast<const unsigned char*>(jpegData.constData()) + jpegData.size()
        );

        m_impl->ws->binary(true);
        // If we fail to queue a write, the write callback will call cleanupConnection()
        m_impl->ws->async_write(asio::buffer(*payload),
            [this, payload](beast::error_code ec, std::size_t bytes_transferred) {
                Q_UNUSED(bytes_transferred);
                if (ec) {
                    qWarning() << "WebSocket async_write error:" << QString::fromStdString(ec.message());
                    cleanupConnection();
                }
            }
        );

        return true;
    } catch (const std::exception &ex) {
        qWarning() << "sendFrame exception:" << ex.what();
        return false;
    }
}

bool WebSocketImageClient::sendControlMessage(const QByteArray &serialized)
{
    if (!m_impl->running.load() || !m_impl->ws || !m_impl->ws->is_open()) {
        m_impl->running.store(false);
        return false;
    }

    try {
        // Prepend control prefix 0x01
        QByteArray withPrefix;
        withPrefix.append(char(0x01));
        withPrefix.append(serialized);

        auto payload = std::make_shared<std::vector<unsigned char>>(
            reinterpret_cast<const unsigned char*>(withPrefix.constData()),
            reinterpret_cast<const unsigned char*>(withPrefix.constData()) + withPrefix.size()
        );

        m_impl->ws->binary(true);
        m_impl->ws->async_write(asio::buffer(*payload),
            [this, payload](beast::error_code ec, std::size_t bytes_transferred) {
                Q_UNUSED(bytes_transferred);
                if (ec) {
                    qWarning() << "WebSocket control async_write error:" << QString::fromStdString(ec.message());
                    cleanupConnection();
                }
            }
        );
        return true;
    } catch (const std::exception &ex) {
        qWarning() << "sendControlMessage exception:" << ex.what();
        return false;
    }
}
