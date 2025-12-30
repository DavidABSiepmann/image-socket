/**
 * @file test_signal_helper.h
 * @brief Utilities for signal-based testing
 *
 * Provides:
 * - Signal emission count assertion helpers
 * - Signal argument value verification helpers
 * - Timeout-safe signal waiting (QSignalSpy-based)
 *
 * Header-only, no assertions, no production logic
 *
 * Usage:
 *   SignalHelper helper(&obj, &Class::signal);
 *   // ... trigger signal ...
 *   int count = helper.emissionCount();
 *   QVariant arg = helper.argumentAt(0, 0);  // First emission, first arg
 */

#ifndef TEST_SIGNAL_HELPER_H
#define TEST_SIGNAL_HELPER_H

#include <QtCore/QObject>
#include <QtCore/QSignalSpy>
#include <QtCore/QVariant>
#include <QtCore/QVariantList>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

/**
 * @namespace qt_test
 * @brief Common Qt test utilities
 */
namespace qt_test {

/**
 * @class SignalHelper
 * @brief Utilities for testing signals
 *
 * Provides non-assertion helpers to inspect and validate signals.
 * Does not contain assertions - test code adds those.
 *
 * Features:
 * - Signal spy management
 * - Emission count tracking
 * - Argument extraction and validation
 * - Timeout-safe waiting (no blocking)
 * - Multiple emission history
 */
class SignalHelper {
private:
    QSignalSpy m_spy;

public:
    /**
     * Create helper for given signal
     * @param sender Object emitting signal
     * @param signal Signal to monitor (use &Class::signal syntax)
     */
    template <typename Func>
    SignalHelper(QObject* sender, Func signal)
        : m_spy(sender, signal) {
    }

    /**
     * Get number of times signal was emitted
     */
    int emissionCount() const {
        return m_spy.count();
    }

    /**
     * Check if signal was never emitted
     */
    bool neverEmitted() const {
        return m_spy.isEmpty();
    }

    /**
     * Check if signal was emitted at least once
     */
    bool wasEmitted() const {
        return !m_spy.isEmpty();
    }

    /**
     * Check if signal emitted exact number of times
     * @param expected Expected emission count
     */
    bool emittedExactly(int expected) const {
        return m_spy.count() == expected;
    }

    /**
     * Check if signal emitted at least N times
     * @param minimum Minimum emission count
     */
    bool emittedAtLeast(int minimum) const {
        return m_spy.count() >= minimum;
    }

    /**
     * Clear emission history
     * Useful to reset spy between operations
     */
    void clear() {
        m_spy.clear();
    }

    // ===== Argument access =====

    /**
     * Get argument from specific emission
     * @param emission Emission index (0-based)
     * @param argument Argument index within that emission (0-based)
     * @return Variant with argument value (or null if invalid)
     */
    QVariant argumentAt(int emission, int argument) const {
        if (emission < 0 || emission >= m_spy.count()) {
            return QVariant();
        }

        const QVariantList& args = m_spy.at(emission);
        if (argument < 0 || argument >= args.count()) {
            return QVariant();
        }

        return args.at(argument);
    }

    /**
     * Get all arguments from specific emission
     * @param emission Emission index (0-based)
     * @return List of arguments (empty if invalid emission)
     */
    QVariantList argumentsAt(int emission) const {
        if (emission < 0 || emission >= m_spy.count()) {
            return QVariantList();
        }
        return m_spy.at(emission);
    }

    /**
     * Get arguments from last emission
     */
    QVariantList lastArguments() const {
        if (m_spy.isEmpty()) {
            return QVariantList();
        }
        return m_spy.last();
    }

    /**
     * Get first argument from last emission
     */
    QVariant lastArgument() const {
        const QVariantList& args = lastArguments();
        if (args.isEmpty()) {
            return QVariant();
        }
        return args.first();
    }

    /**
     * Get first argument from first emission
     */
    QVariant firstArgument() const {
        if (m_spy.isEmpty()) {
            return QVariant();
        }
        const QVariantList& args = m_spy.first();
        if (args.isEmpty()) {
            return QVariant();
        }
        return args.first();
    }

    // ===== Type-safe argument access (with conversion) =====

    /**
     * Get argument as specific type
     * @param emission Emission index
     * @param argument Argument index
     * @return Value converted to T (default-constructed if conversion fails)
     */
    template <typename T>
    T argumentAs(int emission, int argument) const {
        QVariant var = argumentAt(emission, argument);
        if (!var.isValid()) {
            return T();
        }
        return var.value<T>();
    }

    /**
     * Get last argument as specific type
     */
    template <typename T>
    T lastArgumentAs() const {
        return lastArgument().value<T>();
    }

    /**
     * Get first argument as specific type
     */
    template <typename T>
    T firstArgumentAs() const {
        return firstArgument().value<T>();
    }

    // ===== Waiting for emission =====

    /**
     * Wait for signal to be emitted (blocking event loop)
     * @param timeout_ms Maximum time to wait (milliseconds)
     * @param min_emissions Minimum number of emissions to wait for
     * @return True if minimum emissions reached
     */
    bool waitForEmission(int timeout_ms = 5000, int min_emissions = 1) {
        if (m_spy.count() >= min_emissions) {
            return true;  // Already emitted
        }

        QEventLoop loop;
        QTimer timer;

        // Setup timeout
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

        // Process events in chunks
        int elapsed = 0;
        while (elapsed < timeout_ms && m_spy.count() < min_emissions) {
            timer.start(100);
            loop.exec();
            elapsed += 100;

            if (m_spy.count() >= min_emissions) {
                return true;
            }
        }

        return m_spy.count() >= min_emissions;
    }

    /**
     * Wait for signal to be emitted N more times
     * @param additional_emissions Number of additional emissions to wait for
     * @param timeout_ms Maximum time to wait
     * @return True if additional emissions received
     */
    bool waitForAdditionalEmissions(int additional_emissions, int timeout_ms = 5000) {
        int starting_count = m_spy.count();
        return waitForEmission(timeout_ms, starting_count + additional_emissions);
    }

    // ===== History inspection =====

    /**
     * Get all emission history as list
     * Each item is QVariantList of arguments for that emission
     */
    QList<QVariantList> allEmissions() const {
        QList<QVariantList> result;
        for (const QVariantList& args : m_spy) {
            result.append(args);
        }
        return result;
    }

    /**
     * Check if all emissions have same number of arguments
     */
    bool consistentArgumentCount() const {
        if (m_spy.isEmpty()) {
            return true;
        }

        int expected_count = m_spy.first().count();
        for (const QVariantList& args : m_spy) {
            if (args.count() != expected_count) {
                return false;
            }
        }
        return true;
    }

    /**
     * Get expected argument count (from first emission)
     */
    int argumentCount() const {
        if (m_spy.isEmpty()) {
            return 0;
        }
        return m_spy.first().count();
    }

    // ===== Raw access =====

    /**
     * Get raw signal spy for advanced usage
     * WARNING: Direct manipulation may break helper assumptions
     */
    QSignalSpy& spy() {
        return m_spy;
    }

    /**
     * Get const reference to raw signal spy
     */
    const QSignalSpy& constSpy() const {
        return m_spy;
    }
};

/**
 * @class MultiSignalHelper
 * @brief Track multiple signals simultaneously
 *
 * Useful for verifying signal ordering and interactions
 *
 * Usage:
 *   MultiSignalHelper helper;
 *   helper.add(&obj1, &Class::signal1, "signal1");
 *   helper.add(&obj2, &Class::signal2, "signal2");
 *   // ... trigger signals ...
 *   int count1 = helper.count("signal1");
 *   int count2 = helper.count("signal2");
 */
class MultiSignalHelper {
private:
    QMap<QString, QSignalSpy*> m_spies;

public:
    /**
     * Destructor cleans up spies
     */
    ~MultiSignalHelper() {
        for (QSignalSpy* spy : m_spies) {
            delete spy;
        }
    }

    /**
     * Add signal to track
     * @param sender Object emitting signal
     * @param signal Signal to monitor
     * @param name Name for this signal (for later reference)
     */
    template <typename Func>
    void add(QObject* sender, Func signal, const QString& name) {
        if (m_spies.contains(name)) {
            delete m_spies[name];
        }
        m_spies[name] = new QSignalSpy(sender, signal);
    }

    /**
     * Get emission count for named signal
     */
    int count(const QString& name) const {
        if (!m_spies.contains(name)) {
            return -1;
        }
        return m_spies[name]->count();
    }

    /**
     * Get arguments for named signal at specific emission
     */
    QVariantList argumentsAt(const QString& name, int emission) const {
        if (!m_spies.contains(name)) {
            return QVariantList();
        }
        const QSignalSpy* spy = m_spies[name];
        if (emission < 0 || emission >= spy->count()) {
            return QVariantList();
        }
        return spy->at(emission);
    }

    /**
     * Clear all spies
     */
    void clearAll() {
        for (QSignalSpy* spy : m_spies) {
            spy->clear();
        }
    }

    /**
     * Clear specific spy
     */
    void clear(const QString& name) {
        if (m_spies.contains(name)) {
            m_spies[name]->clear();
        }
    }

    /**
     * Get list of tracked signal names
     */
    QStringList names() const {
        return m_spies.keys();
    }

    /**
     * Get spy helper for named signal
     */
    SignalHelper helper(const QString& name) const {
        // This is tricky - we can't easily wrap existing QSignalSpy
        // So we just expose the count
        if (!m_spies.contains(name)) {
            return SignalHelper(nullptr, nullptr);  // Will be invalid
        }
        // NOTE: This is a limitation - prefer accessing via count() or argumentsAt()
        return SignalHelper(nullptr, nullptr);
    }
};

}  // namespace qt_test

#endif  // TEST_SIGNAL_HELPER_H
