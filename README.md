# Document Search Engine

A C++17 document retrieval engine combining inverted indexing, LRU caching, and request rate limiting. The project brings together search algorithms and request-handling components in a command-line application.

## Purpose

Given a query, the engine retrieves the most relevant document from a local text collection based on shared query terms. An inverted index maps words to document IDs, allowing search to focus on documents that contain those terms.

The architecture is inspired by the retrieval stage of Retrieval-Augmented Generation (RAG), with a focus on keyword matching, caching, and traffic control.

## Architecture

Each query passes through rate limiting before the cache is checked. A cache hit returns the stored result. On a cache miss, the search engine scores candidate documents, selects the best match, and caches the response.

| Component | Responsibility |
| --- | --- |
| Inverted index | Map normalized words to document IDs for candidate lookup. |
| Document ranking | Score candidates by the number of distinct query terms they contain. |
| LRU cache | Reuse previous search results and evict the least recently used entry when full. |
| Rate limiter | Control requests using token bucket, leaky bucket, or fixed window algorithms. |
| Command-line interface | Accept queries and display results, cache activity, and rate-limit decisions. |

## Technical foundation

- **C++17** with standard library containers.
- **Hash maps and sets** for indexing, query-term deduplication, and request tracking.
- **A doubly linked list and hash map** for average O(1) cache lookup and insertion.
- **A monotonic clock** for elapsed-time calculations in rate limiting.
- **Text-based document storage**, with one document per line.

## Repository status

The repository currently contains the project overview and ignore rules. Source code will be added in stages: core retrieval, cache integration, and rate limiting, followed by tests and usage documentation.
