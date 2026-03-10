#include "Helper.cpp"
#include "test.cpp"

int main()
{
    std::vector<std::string> possible_words = load_data("words_file.txt");

    std::string guess = "crane";
    std::string target = "slate";
    int feedback_pattern = translate_to_pattern(feedback(guess, target));
    std::vector<std::string> results = filter_words(possible_words, guess, feedback_pattern);
    auto best_words = best_words_to_guess(possible_words);
    print_words(best_words);
    return 0;
}
