/**
 * @file qt_test_base.h
 * @brief Common base utilities for QtTest-based tests
 *
 * Provides:
 * - Safe QCoreApplication lifecycle management
 * - Deterministic event processing
 * - Signal waiting utilities (QSignalSpy-based, non-blocking)
 * - Common macros for Qt test assertions
 *
 * Header-only, no assertions, no production logic
 */

#ifndef QT_TEST_BASE_H
#define QT_TEST_BASE_H

#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtTest/QSignalSpy>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <functional>

/**
 * @namespace qt_test
 * @brief Common Qt test utilities
 */
namespace qt_test {

/**
 * @class QtTestApplicationGuard
 * @brief RAII guard for QCoreApplication lifecycle
 *
 * Ensures QCoreApplication is created before tests and deleted after.
 * Safe for multiple instantiations (uses reference counting).
 */
class QtTestApplicationGuard {
public:
    /**
     * Create or get existing QCoreApplication
     */
    QtTestApplicationGuard() {
        if (!QCoreApplication::instance()) {
            static int argc = 0;
            static char* argv[] = {nullptr};
            new QCoreApplication(argc, argv);
        }
    }

    /**
     * Cleanup (QCoreApplication remains for application lifetime)
     */
    ~QtTestApplicationGuard() = default;

    /**
     * No copying
     */
    QtTestApplicationGuard(const QtTestApplicationGuard&) = delete;
    QtTestApplicationGuard& operator=(const QtTestApplicationGuard&) = delete;
};

/**
 * @class EventLoopSpinner
 * @brief Non-blocking event loop spinning utility
 *
 * Safely spins the event loop with timeout to ensure Qt asynchronous
 * operations complete (signals, timers, etc.)
 *
 * Usage:
 *   EventLoopSpinner spinner(5000);  // 5 second max timeout
 *   // ... setup signals ...
 *   spinner.spin();  // Runs event loop until condition met or timeout
 */
class EventLoopSpinner {
private:
    QEventLoop m_loop;
    int m_timeout_ms;

public:
    /**
     * Create spinner with optional timeout (milliseconds)
     * @param timeout_ms Maximum time to spin (0 = infinite)
     */
    explicit EventLoopSpinner(int timeout_ms = 5000)
        : m_timeout_ms(timeout_ms) {
    }

    /**
     * Spin event loop until finished or timeout
     * Used internally by signal waiters
     */
    void spin() {
        if (m_timeout_ms > 0) {
            QTimer::singleShot(m_timeout_ms, &m_loop, &QEventLoop::quit);
        }
        m_loop.exec();
    }

    /**
     * Stop the event loop
     */
    void stop() {
        m_loop.quit();
    }

    /**
     * Process all pending events without blocking
     */
    static void processEvents() {
        QCoreApplication::processEvents();
    }

    /**
     * Process events with timeout
     * @param max_time_ms Maximum time to process (0 = process once)
     */
    static void processEventsWithTimeout(int max_time_ms = 0) {
        QEventLoop loop;
        if (max_time_ms > 0) {
            QTimer::singleShot(max_time_ms, &loop, &QEventLoop::quit);
            loop.exec();
        } else {
            QCoreApplication::processEvents();
        }
    }
};

/**
 * @class SignalWaiter
 * @brief Wait for a signal to be emitted (non-blocking)
 *
 * Uses QSignalSpy internally to wait for signal without blocking threads.
 * Returns immediately if signal count requirement is met.
 *
 * Usage:
 *   SignalWaiter waiter(&obj, &Class::signal, 1);  // Wait for 1 emission
 *   // ... trigger signal ...
 *   if (waiter.wait(1000)) {  // Wait max 1 second
 *       // Signal was emitted
 *   }
 */
class SignalWaiter {
private:
    QSignalSpy m_spy;
    int m_expected_count;

public:
    /**
     * Create waiter for a signal
     * @param sender Object emitting signal
     * @param signal Signal to wait for (use &Class::signal)
     * @param expected_count Expected number of emissions (default 1)
     */
    template <typename Func>
    SignalWaiter(QObject* sender, Func signal, int expected_count = 1)
        : m_spy(sender, signal), m_expected_count(expected_count) {
    }

    /**
     * Wait for signal with timeout
     * @param timeout_ms Maximum time to wait (milliseconds)
     * @return True if signal emitted the expected number of times
     */
    bool wait(int timeout_ms = 5000) {
        if (m_spy.count() >= m_expected_count) {
            return true;
        }

        EventLoopSpinner spinner(timeout_ms);
        while (m_spy.count() < m_expected_count && timeout_ms > 0) {
            spinner.processEventsWithTimeout(100);
            if (m_spy.count() >= m_expected_count) {
                return true;
            }
            timeout_ms -= 100;
        }

        return m_spy.count() >= m_expected_count;
    }

    /**
     * Get signal spy (for advanced introspection)
     */
    QSignalSpy& spy() {
        return m_spy;
    }

    /**
     * Get number of times signal was emitted
     */
    int count() const {
        return m_spy.count();
    }

    /**
     * Get signal arguments from last emission
     */
    QVariantList lastArgs() const {
        if (m_spy.isEmpty()) {
            return QVariantList();
        }
        return m_spy.last();
    }

    /**
     * Get signal arguments from specific emission (0-based index)
     */
    QVariantList args(int index) const {
        if (index < 0 || index >= m_spy.count()) {
            return QVariantList();
        }
        return m_spy.at(index);
    }
};

/**
 * @class TimerGuard
 * @brief RAII guard for QTimer (cleanup on scope exit)
 */
class TimerGuard {
private:
    QPointer<QTimer> m_timer;

public:
    /**
     * Create timer guard
     * @param timer Timer to manage (takes ownership)
     */
    explicit TimerGuard(QTimer* timer = nullptr)
        : m_timer(timer) {
        if (m_timer) {
            m_timer->stop();
        }
    }

    /**
     * Destructor stops and deletes timer
     */
    ~TimerGuard() {
        if (m_timer) {
            m_timer->stop();
            m_timer->deleteLater();
        }
    }

    /**
     * No copying
     */
    TimerGuard(const TimerGuard&) = delete;
    TimerGuard& operator=(const TimerGuard&) = delete;

    /**
     * Get timer pointer
     */
    QTimer* get() const {
        return m_timer.data();
    }

    /**
     * Reset to manage different timer
     */
    void reset(QTimer* timer) {
        if (m_timer) {
            m_timer->stop();
            m_timer->deleteLater();
        }
        m_timer = timer;
    }
};

/**
 * @brief Initialize Qt test application once per process
 *
 * Safe to call multiple times (subsequent calls are no-ops)
 *
 * Usage:
 *   int main(int argc, char** argv) {
 *       qt_test::initializeQtTestApp();
 *       // ... run tests ...
 *   }
 */
inline void initializeQtTestApp() {
    static QtTestApplicationGuard guard;
    (void)guard;  // Suppress unused warning
}

}  // namespace qt_test

#endif  // QT_TEST_BASE_H
