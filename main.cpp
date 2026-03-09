#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#define WORD_LENGTH 5

std::vector<int> feedback(std::string guess, std::string target);

int main()
{
    std::string word1 = "crane";
    std::string word2 = "slate";
    auto results = feedback(word2, word1);
    for(auto i : results)
    {
        std::cout << i << " ";
    }
    return 0;
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
