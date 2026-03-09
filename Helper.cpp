#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <cmath>
#include <algorithm>

#define WORD_LENGTH 5

std::vector<std::string> load_data(std::string filename)
{
    std::vector<std::string> data;
    // Implementation for loading data from file
    std::ifstream wordsFile(filename);
    if (!wordsFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return std::vector<std::string>();
    }
    std::string word;
    while (wordsFile >> word) {
        data.push_back(word);
    }
    wordsFile.close();
    return data;
}

/*
 * This function returns a feedback for a guess with the following format:
 * 0 - gray (letter not in target)
 * 1 - yellow (letter in target but not in correct location)
 * 2 - green (letter in target and in correct location)
 */
std::vector<int> feedback(const std::string& guess, const std::string& target)
{
    auto result = std::vector<int>(5);
    std::unordered_map<char, int> counts;

    for(auto letter: target) counts[letter]++;

    // Mark greens:
    for(auto i = 0; i < WORD_LENGTH; i++)
    {
        if(guess[i] == target[i])
        {
            result[i] = 2;
            counts[guess[i]]--;
        }
    }
    // Mark yellows:
    for(auto i = 0; i < WORD_LENGTH; i++)
    {
        if(result[i] == 0 && counts[guess[i]] > 0)
        {
            result[i] = 1;
            counts[guess[i]]--;
        }
    }
    return result;
}

int translate_to_pattern(std::vector<int> feedback)
{
    auto result = 0;
    for(int i = 0; i < WORD_LENGTH; i++)
    {
        result += pow(3, i) * feedback[i];
    }
    return result;
}

std::vector<std::string> filter_words(const std::vector<std::string>& possible_words,
    const std::string& guess, int feedback_pattern)
{
    std::vector<std::string> filtered_words;
    for(const auto word : possible_words)
    {
        if(translate_to_pattern(feedback(guess, word)) == feedback_pattern)
        {
            filtered_words.push_back(word);
        }
    }
    return filtered_words;
}

int get_entropy(const std::vector<std::string>& possible_words, const std::string& guess)
{
    double entropy = 0;
    for(int pattern = 0; pattern < 243; pattern++)
    {
        auto filtered_words = filter_words(possible_words, guess, pattern);
        double p = filtered_words.size()/possible_words.size();
        entropy += p*filtered_words.size();
    }
    return entropy;
}

std::vector<std::pair<std::string, int>> best_words_to_guess(const std::vector<std::string>& possible_words)
{
    std::vector<std::pair<std::string, int>> best_words;
    for(auto word : possible_words)
    {
        best_words.push_back({word, get_entropy(possible_words, word)});
    }

    std::sort(best_words.begin(), best_words.end(),
        [](const std::pair<std::string,int>& a, const std::pair<std::string,int>& b) {
            return a.second > b.second;
        });

    return best_words;
}
