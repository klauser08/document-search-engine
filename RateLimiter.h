#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

// rate limiter with 3 algorithms
// TOKEN_BUCKET
// LEAKY_BUCKET
// FIXED_WINDOW

enum class Algorithm
{
    TOKEN_BUCKET,
    LEAKY_BUCKET,
    FIXED_WINDOW
};

class RateLimiter
{
public:
    using Clock = std::chrono::steady_clock;
    using Now = std::function<Clock::time_point()>;

    // rate means requests/second for buckets and seconds/window for fixed windows.
    // A custom monotonic clock allows deterministic tests without sleeping.
    RateLimiter(Algorithm algo, double capacity, double rate, Now now = Clock::now)
        : algo(algo), capacity(capacity), rate(rate), nowProvider(std::move(now))
    {
        if (!std::isfinite(capacity) || capacity < 1.0 ||
            capacity > std::numeric_limits<int>::max() || std::floor(capacity) != capacity)
            throw std::invalid_argument("Rate limiter capacity must be a positive integer");
        if (!std::isfinite(rate) || rate <= 0.0)
            throw std::invalid_argument("Rate or window duration must be positive and finite");
        if (algo != Algorithm::TOKEN_BUCKET && algo != Algorithm::LEAKY_BUCKET &&
            algo != Algorithm::FIXED_WINDOW)
            throw std::invalid_argument("Unknown rate limiting algorithm");
        if (!nowProvider)
            throw std::invalid_argument("A clock provider is required");
    }

    bool allowRequest(const std::string &userId)
    {
        if (algo == Algorithm::TOKEN_BUCKET)
            return allowTokenBucket(userId);
        if (algo == Algorithm::LEAKY_BUCKET)
            return allowLeakyBucket(userId);
        return allowFixedWindow(userId);
    }

private:
    Algorithm algo;
    double capacity;
    double rate;
    Now nowProvider;

    // bucket fills up to capacity over time, each request costs 1 token
   
    struct TokenBucketState
    {
        double tokens;
        std::chrono::steady_clock::time_point lastRefillTime;
    };
    std::unordered_map<std::string, TokenBucketState> tokenBuckets;

    bool allowTokenBucket(const std::string &userId)
    {
        auto now = nowProvider();

        if (tokenBuckets.find(userId) == tokenBuckets.end())
            tokenBuckets[userId] = {capacity, now};

        auto &state = tokenBuckets[userId];

        double elapsed = std::chrono::duration<double>(now - state.lastRefillTime).count();
        state.tokens = std::min(capacity, state.tokens + elapsed * rate);
        state.lastRefillTime = now;

        if (state.tokens >= 1.0)
        {
            state.tokens -= 1.0;
            return true;
        }

        return false;
    }

    // water level increases by 1 per request, drains at fixed rate
    // Admission control: allows bursts up to capacity; does not queue requests.
    struct LeakyBucketState
    {
        double level;
        std::chrono::steady_clock::time_point lastLeakTime;
    };
    std::unordered_map<std::string, LeakyBucketState> leakyBuckets;

    bool allowLeakyBucket(const std::string &userId)
    {
        auto now = nowProvider();

        if (leakyBuckets.find(userId) == leakyBuckets.end())
            leakyBuckets[userId] = {0.0, now};

        auto &state = leakyBuckets[userId];

        double elapsed = std::chrono::duration<double>(now - state.lastLeakTime).count();
        state.level = std::max(0.0, state.level - elapsed * rate);
        state.lastLeakTime = now;

        if (state.level + 1.0 <= capacity)
        {
            state.level += 1.0;
            return true;
        }

        return false;
    }

    // counts requests in current window, resets when window expires
    // can allow up to 2x capacity near window boundaries
    struct FixedWindowState
    {
        int count;
        std::chrono::steady_clock::time_point windowStart;
    };
    std::unordered_map<std::string, FixedWindowState> fixedWindows;

    bool allowFixedWindow(const std::string &userId)
    {
        auto now = nowProvider();

        if (fixedWindows.find(userId) == fixedWindows.end())
            fixedWindows[userId] = {0, now};

        auto &state = fixedWindows[userId];

        double elapsed = std::chrono::duration<double>(now - state.windowStart).count();

        if (elapsed >= rate)
        {
            state.count = 0;
            state.windowStart = now;
        }

        if (state.count < (int)capacity)
        {
            state.count++;
            return true;
        }

        return false;
    }
};
