#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>

#define WORD_LENGTH 5
#define NUM_OF_WORDS 15000

std::vector<std::string> load_data(std::string filename)
{
    std::vector<std::string> data;
    int number_to_load = NUM_OF_WORDS;
    // Implementation for loading data from file
    std::ifstream wordsFile(filename);
    if (!wordsFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return std::vector<std::string>();
    }
    std::string word;
    while (wordsFile >> word && number_to_load > 0) {
        data.push_back(word);
        number_to_load--;
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
    int counts[26] = {0};

    for(auto letter: target) counts[letter - 'a']++;

    // Mark greens:
    for(auto i = 0; i < WORD_LENGTH; i++)
    {
        if(guess[i] == target[i])
        {
            result[i] = 2;
            counts[guess[i] - 'a']--;
        }
    }
    // Mark yellows:
    for(auto i = 0; i < WORD_LENGTH; i++)
    {
        if(result[i] == 0 && counts[guess[i] - 'a'] > 0)
        {
            result[i] = 1;
            counts[guess[i] - 'a']--;
        }
    }
    return result;
}

int translate_to_pattern(std::vector<int> feedback)
{
    int result = 0;
        int multiplier = 1;
        for(int i = 0; i < WORD_LENGTH; i++)
        {
            result += multiplier * feedback[i];
            multiplier *= 3;
        }
        return result;
}

std::vector<std::string> filter_words(const std::vector<std::string>& possible_words,
    const std::string& guess, int feedback_pattern)
{
    std::vector<std::string> filtered_words;
    for(const auto& word : possible_words)
    {
        if(translate_to_pattern(feedback(guess, word)) == feedback_pattern)
        {
            filtered_words.push_back(word);
        }
    }
    return filtered_words;
}

double get_entropy(const std::vector<std::string>& possible_words, const std::string& guess) {
    std::vector<int> pattern_counts(243, 0);
    for (const auto& target : possible_words) {
        int pattern = translate_to_pattern(feedback(guess, target));
        pattern_counts[pattern]++;
    }

    double entropy = 0;
    double total_words = possible_words.size();
    for (int count : pattern_counts) {
        if (count > 0) {
            double p = count / total_words;
            entropy -= p * log2(p);
        }
    }
    return entropy;
}

std::vector<std::pair<std::string, double>> best_words_to_guess(const std::vector<std::string>& possible_words)
{
    std::vector<std::pair<std::string, double>> best_words;
    for(const auto& word : possible_words)
    {
        best_words.push_back({word, get_entropy(possible_words, word)});
    }

    std::sort(best_words.begin(), best_words.end(),
        [](const std::pair<std::string,double>& a, const std::pair<std::string,double>& b) {
            return a.second > b.second;
        });

    return best_words;
}
