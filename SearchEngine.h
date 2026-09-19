#pragma once

#include <cctype>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// loads documents.txt and finds the best matching document for a query
// uses an inverted index built at load time for efficient retrieval
//
// inverted index maps each word -> list of document indices that contain it
// so instead of scanning all documents per query, we only look at relevant ones
//
// example:
//   "binary" -> [0, 4, 7]
//   "search" -> [0, 2, 4]
//   "tree"   -> [0, 6]

class SearchEngine
{
public:
    bool loadDocuments(const std::string &filePath)
    {
        std::ifstream file(filePath);

        if (!file.is_open())
            return false;

        // A successful load replaces the previous collection.
        documents.clear();
        invertedIndex.clear();

        std::string line;
        while (getline(file, line))
        {
            if (!tokenize(line).empty())
            {
                int docId = (int)documents.size();
                documents.push_back(line);
                buildIndex(line, docId);
            }
        }

        return true;
    }

    // returns {bestDoc, score}
    // only looks at documents that share at least one word with the query
    std::pair<std::string, int> search(const std::string &query) const
    {
        std::unordered_set<std::string> queryWords = tokenize(query);

        // gather candidate document ids using inverted index
        std::unordered_map<int, int> scores;

        for (const std::string &word : queryWords)
        {
            auto it = invertedIndex.find(word);
            if (it != invertedIndex.end())
            {
                for (int docId : it->second)
                    scores[docId]++;
            }
        }

        // find highest scoring document
        std::string bestDoc = "";
        int bestScore = 0;
        int bestDocId = -1;

        for (auto &[docId, score] : scores)
        {
            if (score > bestScore || (score == bestScore && docId < bestDocId))
            {
                bestScore = score;
                bestDocId = docId;
                bestDoc = documents[docId];
            }
        }

        return {bestDoc, bestScore};
    }

    int documentCount() const
    {
        return (int)documents.size();
    }

private:
    std::vector<std::string> documents;
    std::unordered_map<std::string, std::vector<int>> invertedIndex;

    // lowercases and splits text into a set of words
    static std::unordered_set<std::string> tokenize(const std::string &text)
    {
        std::unordered_set<std::string> words;
        std::string cur;

        for (char c : text)
        {
            if (std::isalnum((unsigned char)c))
            {
                cur += (char)std::tolower((unsigned char)c);
            }
            else
            {
                if (!cur.empty())
                {
                    words.insert(cur);
                    cur.clear();
                }
            }
        }

        if (!cur.empty())
            words.insert(cur);

        return words;
    }

    // called once per document at load time
    // maps every word in the document to this document's id
    void buildIndex(const std::string &doc, int docId)
    {
        std::unordered_set<std::string> words = tokenize(doc);

        for (const std::string &word : words)
            invertedIndex[word].push_back(docId);
    }
};
