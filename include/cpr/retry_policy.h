#ifndef CPR_RETRY_POLICY_H
#define CPR_RETRY_POLICY_H

#include <functional>
#include <chrono>
#include "cpr/error.h"
#include "cpr/status_codes.h"

namespace cpr {

class RetryPolicy {
public:
    virtual ~RetryPolicy() = default;
    
    virtual bool ShouldRetry(const Response& response, const Error& error) const = 0;
    virtual std::chrono::milliseconds GetRetryDelay(int retry_count) const = 0;
};

class ExponentialBackoffRetryPolicy : public RetryPolicy {
public:
    ExponentialBackoffRetryPolicy(int max_retries, std::chrono::milliseconds initial_delay,
                                 std::chrono::milliseconds max_delay = std::chrono::seconds(30),
                                 double multiplier = 2.0);
    
    bool ShouldRetry(const Response& response, const Error& error) const override;
    std::chrono::milliseconds GetRetryDelay(int retry_count) const override;
    
    void AddRetryStatusCode(int status_code);
    void AddRetryError(ErrorCode error_code);
    
private:
    int max_retries_;
    std::chrono::milliseconds initial_delay_;
    std::chrono::milliseconds max_delay_;
    double multiplier_;
    
    std::vector<int> retry_status_codes_;
    std::vector<ErrorCode> retry_error_codes_;
};

} // namespace cpr

#endif // CPR_RETRY_POLICY_H