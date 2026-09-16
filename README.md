# Document Search Engine

A small C++17 command-line project for searching a local collection of documents.

The idea is to explore the retrieval part of a RAG pipeline: find a document that matches a query using an inverted index. The project also includes an LRU cache for repeated queries and a few rate-limiting algorithms. It does not use an LLM or generate answers.

## Repository status

This is the initial repository setup. Source code and build instructions will be added next.

## Planned additions

- Load documents from a text file and rank matches by shared query words.
- Cache search results with an LRU eviction policy.
- Compare token bucket, leaky bucket, and fixed window rate limiting.
- Add tests for search, caching, and request limits.
