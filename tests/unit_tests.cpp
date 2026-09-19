#include "LRUCache.h"
#include "RateLimiter.h"
#include "SearchEngine.h"

#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
int checks = 0;

void check(bool condition, const char *message)
{
    ++checks;
    if (!condition)
        throw std::runtime_error(message);
}

template <typename Action>
void rejects(Action action, const char *message)
{
    bool rejected = false;
    try
    {
        action();
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    check(rejected, message);
}

void testSearch()
{
    SearchEngine engine;
    check(engine.search("binary").second == 0, "empty engine has no match");
    check(engine.loadDocuments("tests/documents.txt"), "load search fixture");
    check(engine.documentCount() == 3, "ignore lines without searchable words");
    auto result = engine.search("BINARY, search TREE!");
    check(result.first == "Binary search tree" && result.second == 3,
          "normalize case and punctuation, rank by matching terms");
    check(engine.search("binary binary").second == 1, "deduplicate query terms");
    check(engine.search("cache").second == 1, "deduplicate document terms");
    check(engine.search("binary search").first == "Binary search tree",
          "break ties using document order");
    for (const auto &query : {"", "!!!", "unknownword"})
    {
        result = engine.search(query);
        check(result.first.empty() && result.second == 0, "no-match result");
    }
    check(engine.loadDocuments("tests/documents.txt") && engine.documentCount() == 3,
          "reload replaces the collection");
    check(!engine.loadDocuments("tests/missing/documents.txt"), "missing file fails");
    check(engine.documentCount() == 3 && engine.search("tree").second == 1,
          "failed open preserves existing collection");
}

void testCache()
{
    rejects([] { LRUCache cache(0); }, "reject zero capacity");
    rejects([] { LRUCache cache(-1); }, "reject negative capacity");
    LRUCache cache(2);
    std::string value;
    check(!cache.get("missing", value), "cache miss");
    cache.put("a", "first");
    cache.put("b", "second");
    check(cache.get("a", value) && value == "first", "cache hit promotes entry");
    cache.put("c", "third");
    check(!cache.get("b", value), "evict least recently used entry");
    cache.put("a", "updated");
    cache.put("d", "fourth");
    check(!cache.get("c", value), "updating an entry refreshes recency");
    check(cache.get("a", value) && value == "updated", "updated value retained");
    LRUCache single(1);
    single.put("x", "one");
    single.put("y", "two");
    check(!single.get("x", value) && single.get("y", value), "capacity-one eviction");
}

void testRateLimiting()
{
    for (auto algorithm : {Algorithm::TOKEN_BUCKET, Algorithm::LEAKY_BUCKET,
                           Algorithm::FIXED_WINDOW})
    {
        RateLimiter::Clock::time_point now{};
        RateLimiter limiter(algorithm, 3, algorithm == Algorithm::FIXED_WINDOW ? 5.0 : 1.0,
                            [&now] { return now; });
        check(limiter.allowRequest("alice"), "allow first request");
        check(limiter.allowRequest("alice"), "allow second request");
        check(limiter.allowRequest("alice"), "allow third request");
        check(!limiter.allowRequest("alice"), "reject exhausted capacity");
        check(limiter.allowRequest("bob"), "independent allowance per user");
        now += std::chrono::milliseconds(500);
        check(!limiter.allowRequest("alice"), "partial interval does not allow a request");
        if (algorithm == Algorithm::FIXED_WINDOW)
        {
            now += std::chrono::milliseconds(4499);
            check(!limiter.allowRequest("alice"), "window has not ended");
            now += std::chrono::milliseconds(1);
            check(limiter.allowRequest("alice"), "reset at exact window boundary");
            check(limiter.allowRequest("alice") && limiter.allowRequest("alice"),
                  "new window restores full allowance");
            check(!limiter.allowRequest("alice"), "new window still enforces capacity");
        }
        else
        {
            now += std::chrono::milliseconds(500);
            check(limiter.allowRequest("alice"), "one second restores one request");
            check(!limiter.allowRequest("alice"), "recovery restores only earned capacity");
        }
        now += std::chrono::seconds(100);
        for (int i = 0; i < 3; ++i)
            check(limiter.allowRequest("alice"), "idle recovery restores capacity");
        check(!limiter.allowRequest("alice"), "idle recovery is capped");
    }
    for (double capacity : {0.0, -1.0, 1.5, std::numeric_limits<double>::infinity(),
                            std::numeric_limits<double>::quiet_NaN(), 1e20})
        rejects([capacity] { RateLimiter limiter(Algorithm::TOKEN_BUCKET, capacity, 1); },
                "reject invalid capacity");
    for (double rate : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                        std::numeric_limits<double>::quiet_NaN()})
        rejects([rate] { RateLimiter limiter(Algorithm::FIXED_WINDOW, 3, rate); },
                "reject invalid rate or duration");
    rejects([] { RateLimiter limiter(static_cast<Algorithm>(99), 3, 1); },
            "reject invalid algorithm");
    rejects([] { RateLimiter limiter(Algorithm::TOKEN_BUCKET, 3, 1, {}); },
            "reject empty clock provider");
}
} // namespace

int main()
{
    try
    {
        testSearch();
        testCache();
        testRateLimiting();
        std::cout << "Passed " << checks << " unit checks\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
