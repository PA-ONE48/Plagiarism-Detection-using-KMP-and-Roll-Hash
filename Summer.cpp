#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>


using namespace std;

class PlagiarismChecker {
private:
    static const int BASE = 256;
    static const int MOD = 1000000007;

    // Rolling Hash structure
    struct RollingHash {
        vector<long long> hash;
        vector<long long> pow;
        int n;

        RollingHash(const string& s) {
            n = s.length();
            hash.resize(n + 1, 0);
            pow.resize(n + 1, 1);
            for (int i = 0; i < n; i++) {
                hash[i + 1] = (hash[i] * BASE + s[i]) % MOD;
                pow[i + 1] = (pow[i] * BASE) % MOD;
            }
        }

        long long getHash(int l, int r) {
            long long result = hash[r] - (hash[l] * pow[r - l]) % MOD;
            return (result + MOD) % MOD;
        }
    };

    // Text cleaning: lowercase, remove punctuation
    string preprocessText(const string& text) {
        string result;
        for (char c : text) {
            if (isalnum(c)) {
                result += tolower(c);
            } else if (isspace(c)) {
                result += ' ';
            }
        }
        return result;
    }

    // KMP Helper: Build longest prefix-suffix array
    vector<int> computeLPS(const string& pattern) {
        int m = pattern.length();
        vector<int> lps(m, 0);
        int len = 0, i = 1;
        while (i < m) {
            if (pattern[i] == pattern[len]) {
                len++;
                lps[i] = len;
                i++;
            } else {
                if (len != 0) {
                    len = lps[len - 1];
                } else {
                    lps[i] = 0;
                    i++;
                }
            }
        }
        return lps;
    }

    // KMP search to find all matches
    vector<int> KMPSearch(const string& text, const string& pattern) {
        vector<int> matches;
        int n = text.length(), m = pattern.length();
        vector<int> lps = computeLPS(pattern);

        int i = 0, j = 0;
        while (i < n) {
            if (pattern[j] == text[i]) {
                i++;
                j++;
            }
            if (j == m) {
                matches.push_back(i - j);
                j = lps[j - 1];
            } else if (i < n && pattern[j] != text[i]) {
                if (j != 0) {
                    j = lps[j - 1];
                } else {
                    i++;
                }
            }
        }
        return matches;
    }

public:
    struct PlagiarismResult {
        double similarity;
        int rollingMatches;
        int kmpMatches;
        int largestSubstringLength;
        string largestSubstring;

        PlagiarismResult() {
            similarity = 0.0;
            rollingMatches = 0;
            kmpMatches = 0;
            largestSubstringLength = 0;
            largestSubstring = "";
        }
    };

    // Rolling Hash matching
    PlagiarismResult checkWithRollingHash(const string& text1, const string& text2, int minSize = 10) {
        PlagiarismResult result;

        string proc1 = preprocessText(text1);
        string proc2 = preprocessText(text2);

        if (proc1.empty() || proc2.empty()) return result;

        RollingHash rh1(proc1);
        RollingHash rh2(proc2);

        unordered_map<long long, vector<int>> hashMap;

        for (int i = 0; i < (int)proc2.size(); ++i) {
            for (int len = minSize; i + len <= (int)proc2.size(); ++len) {
                long long h = rh2.getHash(i, i + len);
                hashMap[h].push_back(i);
            }
        }

        vector<bool> matched(proc1.size(), false);

        for (int i = 0; i < (int)proc1.size(); ++i) {
            int maxLen = 0;
            string maxSubstring = "";
            for (int len = minSize; i + len <= (int)proc1.size(); ++len) {
                long long h = rh1.getHash(i, i + len);
                if (hashMap.count(h)) {
                    for (int pos : hashMap[h]) {
                        if (proc1.substr(i, len) == proc2.substr(pos, len)) {
                            if (len > maxLen) {
                                maxLen = len;
                                maxSubstring = proc1.substr(i, len);
                            }
                        }
                    }
                }
            }
            if (maxLen > 0) {
                for (int k = i; k < i + maxLen; ++k) matched[k] = true;
                result.rollingMatches++;
                if (maxLen > result.largestSubstringLength) {
                    result.largestSubstringLength = maxLen;
                    result.largestSubstring = maxSubstring;
                }
                i += maxLen - 1;
            }
        }

        int matchedCount = count(matched.begin(), matched.end(), true);
        result.similarity = (double)matchedCount / proc1.size() * 100;

        return result;
    }

    // KMP: match common phrases
    int checkWithKMP(const string& text1, const string& text2, int phraseLength = 3) {
        string proc1 = preprocessText(text1);
        string proc2 = preprocessText(text2);

        vector<string> words;
        stringstream ss(proc1);
        string word;
        while (ss >> word) words.push_back(word);

        unordered_set<string> phrases;
        for (int i = 0; i <= (int)words.size() - phraseLength; ++i) {
            string phrase = "";
            for (int j = 0; j < phraseLength; ++j) {
                phrase += words[i + j];
                if (j < phraseLength - 1) phrase += " ";
            }
            phrases.insert(phrase);
        }

        int totalMatches = 0;
        for (const string& phrase : phrases) {
            auto matches = KMPSearch(proc2, phrase);
            totalMatches += matches.size();
        }
        return totalMatches;
    }

    void displayResults(const PlagiarismResult& result) {
        cout << fixed << setprecision(2);
        cout << "Similarity Percentage: " << result.similarity << "%" << endl;
        cout << "Rolling Hash Substring Matches: " << result.rollingMatches << endl;
        cout << "KMP Phrase Matches: " << result.kmpMatches << endl;
        cout << "Largest Matching Substring Length: " << result.largestSubstringLength << endl;
        cout << "Largest Substring: \"" << result.largestSubstring << "\"" << endl;

        if (result.similarity > 70) {
            cout << "Plagiarism Level: HIGH" << endl;
        } else if (result.similarity > 40) {
            cout << "Plagiarism Level: MODERATE" << endl;
        } else if (result.similarity > 15) {
            cout << "Plagiarism Level: LOW" << endl;
        } else {
            cout << "Plagiarism Level: NONE" << endl;
        }
    }
};

int main() {
    PlagiarismChecker checker;

    string doc1 = "Artificial intelligence and machine learning are transforming the world. They help automate tasks and provide valuable insights in many domains.";
    string doc2 = "Machine learning and artificial intelligence are transforming the world and helping automate tasks. They provide valuable insights for many industries.";

    cout << "=== Plagiarism Checker using Rolling Hash + KMP ===" << endl;

    auto result = checker.checkWithRollingHash(doc1, doc2);
    result.kmpMatches = checker.checkWithKMP(doc1, doc2, 3);

    checker.displayResults(result);

    return 0;
}