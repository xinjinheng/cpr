#include "cpr/retry_policy.h"

namespace cpr {

ExponentialBackoffRetryPolicy::ExponentialBackoffRetryPolicy(int max_retries, std::chrono::milliseconds initial_delay,
                                                             std::chrono::milliseconds max_delay, double multiplier)
    : max_retries_(max_retries), initial_delay_(initial_delay), max_delay_(max_delay), multiplier_(multiplier) {
    // Add default retry status codes
    retry_status_codes_ = {
        static_cast<int>(StatusCode::SERVICE_UNAVAILABLE),
        static_cast<int>(StatusCode::GATEWAY_TIMEOUT),
        static_cast<int>(StatusCode::TOO_MANY_REQUESTS)
    };
    
    // Add default retry error codes
    retry_error_codes_ = {
        ErrorCode::CONNECTION_FAILURE,
        ErrorCode::TIMEOUT,
        ErrorCode::SSL_CONNECT_ERROR,
        ErrorCode::SSL_VERIFYRESULT,
        ErrorCode::PROXY_CONNECTION_FAILED
    };
}

bool ExponentialBackoffRetryPolicy::ShouldRetry(const Response& response, const Error& error) const {
    if (error.code != ErrorCode::OK) {
        return std::find(retry_error_codes_.begin(), retry_error_codes_.end(), error.code) != retry_error_codes_.end();
    }
    
    if (response.status_code != 0) {
        return std::find(retry_status_codes_.begin(), retry_status_codes_.end(), response.status_code) != retry_status_codes_.end();
    }
    
    return false;
}

std::chrono::milliseconds ExponentialBackoffRetryPolicy::GetRetryDelay(int retry_count) const {
    if (retry_count >= max_retries_) {
        return std::chrono::milliseconds::max();
    }
    
    auto delay = static_cast<int64_t>(initial_delay_.count() * std::pow(multiplier_, retry_count));
    return std::min(std::chrono::milliseconds(delay), max_delay_);
}

void ExponentialBackoffRetryPolicy::AddRetryStatusCode(int status_code) {
    retry_status_codes_.push_back(status_code);
}

void ExponentialBackoffRetryPolicy::AddRetryError(ErrorCode error_code) {
    retry_error_codes_.push_back(error_code);
}

} // namespace cpr