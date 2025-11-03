#ifndef CPR_CIRCUIT_BREAKER_H
#define CPR_CIRCUIT_BREAKER_H

#include <chrono>
#include <atomic>
#include <mutex>
#include <deque>
#include "cpr/error.h"
#include "cpr/status_codes.h"

namespace cpr {

enum class CircuitBreakerState {
    CLOSED,
    OPEN,
    HALF_OPEN
};

class CircuitBreaker {
public:
    CircuitBreaker(int failure_threshold, std::chrono::milliseconds reset_timeout,
                   int half_open_max_requests = 1);
    
    bool IsAllowed();
    void RecordSuccess();
    void RecordFailure();
    
    CircuitBreakerState GetState() const;
    
private:
    void TransitionToClosed();
    void TransitionToOpen();
    void TransitionToHalfOpen();
    
    int failure_threshold_;
    std::chrono::milliseconds reset_timeout_;
    int half_open_max_requests_;
    
    std::atomic<CircuitBreakerState> state_;
    std::atomic<int> failure_count_;
    std::atomic<int> half_open_requests_;
    std::chrono::steady_clock::time_point last_failure_time_;
    
    mutable std::mutex mutex_;
};

} // namespace cpr

#endif // CPR_CIRCUIT_BREAKER_H