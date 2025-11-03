#include "cpr/circuit_breaker.h"
#include <algorithm>

namespace cpr {

CircuitBreaker::CircuitBreaker(int failure_threshold, std::chrono::milliseconds reset_timeout,
                               int half_open_max_requests)
    : failure_threshold_(failure_threshold), reset_timeout_(reset_timeout),
      half_open_max_requests_(half_open_max_requests), state_(CircuitBreakerState::CLOSED),
      failure_count_(0), half_open_requests_(0) {
}

bool CircuitBreaker::IsAllowed() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    switch (state_) {
        case CircuitBreakerState::CLOSED:
            return true;
            
        case CircuitBreakerState::OPEN:
            // Check if enough time has passed to transition to HALF_OPEN
            auto now = std::chrono::steady_clock::now();
            if (now - last_failure_time_ >= reset_timeout_) {
                TransitionToHalfOpen();
                return true;
            }
            return false;
            
        case CircuitBreakerState::HALF_OPEN:
            // Allow a limited number of requests in half-open state
            if (half_open_requests_ < half_open_max_requests_) {
                half_open_requests_++;
                return true;
            }
            return false;
    }
    
    return false;
}

void CircuitBreaker::RecordSuccess() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_ == CircuitBreakerState::HALF_OPEN) {
        TransitionToClosed();
    } else if (state_ == CircuitBreakerState::CLOSED) {
        failure_count_ = 0;
    }
}

void CircuitBreaker::RecordFailure() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_ == CircuitBreakerState::CLOSED) {
        failure_count_++;
        if (failure_count_ >= failure_threshold_) {
            TransitionToOpen();
        }
    } else if (state_ == CircuitBreakerState::HALF_OPEN) {
        TransitionToOpen();
    }
}

CircuitBreakerState CircuitBreaker::GetState() const {
    return state_.load();
}

void CircuitBreaker::TransitionToClosed() {
    state_ = CircuitBreakerState::CLOSED;
    failure_count_ = 0;
    half_open_requests_ = 0;
}

void CircuitBreaker::TransitionToOpen() {
    state_ = CircuitBreakerState::OPEN;
    last_failure_time_ = std::chrono::steady_clock::now();
    half_open_requests_ = 0;
}

void CircuitBreaker::TransitionToHalfOpen() {
    state_ = CircuitBreakerState::HALF_OPEN;
    half_open_requests_ = 0;
}

} // namespace cpr