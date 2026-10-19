#include "Helper.cpp"

/*
 * Line-based solver engine, meant to be driven by another program (auto_wordle.py).
 * Protocol:
 *   engine -> "GUESS <word>"           the word to play next
 *   driver -> "<5 digits>"             feedback for that guess (0 gray, 1 yellow, 2 green)
 *   driver -> "INVALID"                the site rejected the word; it is dropped and a new guess is sent
 *   engine -> "SOLVED" / "ERROR <msg>" terminal states
 */

std::string pick_guess(const std::vector<std::string> &words_dataset,
                       const std::vector<std::string> &possible_words)
{
    if (possible_words.size() <= 2)
        return possible_words[0];

    auto ranked = best_words_to_guess(words_dataset, possible_words);
    // among words tied for the best entropy, prefer one that could be the answer
    for (const auto &candidate : ranked)
    {
        if (candidate.second < ranked[0].second - 1e-9)
            break;
        if (std::find(possible_words.begin(), possible_words.end(), candidate.first) != possible_words.end())
            return candidate.first;
    }
    return ranked[0].first;
}

int main()
{
    auto words_dataset = load_data("allowed_words.txt");
    auto possible_words = load_data("target_words.txt");
    std::vector<std::pair<std::string, int>> history;
    bool using_fallback = false;
    if (words_dataset.empty() || possible_words.empty())
    {
        std::cout << "ERROR could not load word lists" << std::endl;
        return 1;
    }

    while (true)
    {
        std::string guess = pick_guess(words_dataset, possible_words);
        std::cout << "GUESS " << guess << std::endl;

        std::string input;
        if (!std::getline(std::cin, input))
            return 0;

        if (input == "INVALID")
        {
            words_dataset.erase(std::remove(words_dataset.begin(), words_dataset.end(), guess), words_dataset.end());
            possible_words.erase(std::remove(possible_words.begin(), possible_words.end(), guess), possible_words.end());
            if (possible_words.empty())
            {
                std::cout << "ERROR no words remain" << std::endl;
                return 1;
            }
            continue;
        }

        if (input == "22222")
        {
            std::cout << "SOLVED" << std::endl;
            return 0;
        }

        history.push_back({guess, parse_feedback(input)});
        possible_words = filter_words(possible_words, guess, history.back().second);
        if (possible_words.empty() && !using_fallback)
        {
            // the answer is not in target_words.txt: replay all feedback against the full allowed list
            using_fallback = true;
            possible_words = words_dataset;
            for (const auto &[past_guess, past_pattern] : history)
                possible_words = filter_words(possible_words, past_guess, past_pattern);
        }
        if (possible_words.empty())
        {
            std::cout << "ERROR no words remain, feedback may be inconsistent" << std::endl;
            return 1;
        }
    }
}
