#include <string>
#include <vector>
#include "Helper.cpp"

std::vector<std::string> filter_words(const std::vector<std::string>& possible_words,
    const std::string& guess, int feedback_pattern)
{
    std::vector<std::string> filtered_words;
    for(const auto word : possible_words)
    {
        if(Helper::feedback(guess, word) == feedback_pattern)
        {
            filtered_words.push_back(word);
        }
    }
    return filtered_words;
}
