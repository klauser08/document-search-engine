#include <iostream>
#include <string>
#include "RateLimiter.h"
#include "LRUCache.h"
#include "SearchEngine.h"

using namespace std;

// flow: query -> rate limiter -> cache check -> search -> cache result -> print

bool testAlgorithm(Algorithm algo, const string &algoName, const string &algoDesc,
                   SearchEngine &searchEngine, LRUCache &cache)
{
    cout << "========================================" << endl;
    cout << "  " << algoName << endl;
    cout << "  " << algoDesc << endl;
    cout << "========================================" << endl;
    cout << "  Type 'next' to switch algorithm or 'exit' to quit.\n" << endl;

    RateLimiter rl(algo, 3, algo == Algorithm::FIXED_WINDOW ? 5.0 : 1.0);
    int reqCount = 0;

    while (true)
    {
        cout << "  Enter query: ";
        string query;
        if (!getline(cin, query) || query == "exit")
            return false;

        if (query == "next")
            break;

        if (query.empty())
            continue;

        reqCount++;
        cout << "  Request " << reqCount << ": ";

        if (!rl.allowRequest("user1"))
        {
            cout << "REJECTED - rate limit hit\n" << endl;
            continue;
        }

        string cachedResult;
        if (cache.get(query, cachedResult))
        {
            cout << "ALLOWED [CACHE HIT]" << endl;
            cout << "  -> " << cachedResult << "\n" << endl;
            continue;
        }

        auto result = searchEngine.search(query);
        string finalResult = (result.second <= 0) ? "No relevant document found." : result.first;

        cache.put(query, finalResult);

        cout << "ALLOWED [CACHE MISS]" << endl;
        cout << "  -> " << finalResult << "\n" << endl;
    }

    cout << endl;
    return true;
}

int main()
{
    SearchEngine searchEngine;
    if (!searchEngine.loadDocuments("documents.txt"))
    {
        cout << "Error: could not open documents.txt" << endl;
        return 1;
    }

    LRUCache cache(5);

    cout << "\n========================================" << endl;
    cout << "    Smart Document Search Engine" << endl;
    cout << "    RAG-Inspired Retrieval System" << endl;
    cout << "========================================" << endl;
    cout << "  Loaded " << searchEngine.documentCount() << " documents." << endl;
    cout << "  Rate Limiter Demo: 3 algorithms, capacity=3\n" << endl;
    cout << "  Try sending 4+ queries in a row to see rate limiting kick in." << endl;
    cout << "  Type 'next' to switch to the next algorithm.\n" << endl;

    if (!testAlgorithm(Algorithm::TOKEN_BUCKET, "1. Token Bucket (capacity=3, refill=1/sec)",
                  "Allows bursts up to capacity, then refills over time.",
                  searchEngine, cache))
    {
        cout << "Goodbye!" << endl;
        return 0;
    }

    if (!testAlgorithm(Algorithm::LEAKY_BUCKET, "2. Leaky Bucket (capacity=3, leak=1/sec)",
                  "Allows bursts up to capacity while the bucket drains over time.",
                  searchEngine, cache))
    {
        cout << "Goodbye!" << endl;
        return 0;
    }

    if (!testAlgorithm(Algorithm::FIXED_WINDOW, "3. Fixed Window (capacity=3, window=5sec)",
                  "Counts requests per window, resets when window expires.",
                  searchEngine, cache))
    {
        cout << "Goodbye!" << endl;
        return 0;
    }

    cout << "========================================" << endl;
    cout << "  Demo complete. Entering normal search mode." << endl;
    cout << "  Type 'exit' to quit." << endl;
    cout << "========================================\n" << endl;

    RateLimiter rateLimiter(Algorithm::TOKEN_BUCKET, 3, 1.0);

    while (true)
    {
        cout << "Enter query: ";
        string query;
        if (!getline(cin, query) || query == "exit")
            break;

        if (query.empty())
            continue;

        if (!rateLimiter.allowRequest("user1"))
        {
            cout << "Rate limit exceeded. Please wait.\n" << endl;
            continue;
        }

        string cachedResult;
        if (cache.get(query, cachedResult))
        {
            cout << "[CACHE HIT] " << cachedResult << "\n" << endl;
            continue;
        }

        auto result = searchEngine.search(query);
        string finalResult = (result.second <= 0) ? "No relevant document found." : result.first;

        cache.put(query, finalResult);
        cout << "[CACHE MISS] " << finalResult << "\n" << endl;
    }

    cout << "Goodbye!" << endl;
    return 0;
}
