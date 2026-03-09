#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <cmath>

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
        result += pow(3, i) * feedback[WORD_LENGTH-i];
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
