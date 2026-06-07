#include "Helper.cpp"
#include "test.cpp"

int main()
{
    auto words_dataset = load_data("words_file.txt");
    auto possible_words = words_dataset;

    while (true)
    {
        auto best_word = best_words_to_guess(words_dataset,possible_words)[0];

        std::cout << "\nSuggested guess: "
                  << best_word.first
                  << " (entropy = "
                  << best_word.second
                  << ")\n";

        std::cout << "Remaining words: "
                  << possible_words.size()
                  << "\n";

        std::cout << "Enter feedback (e.g. 02010): ";

        std::string input;
        std::cin >> input;

        if (input == "22222")
        {
            std::cout << "Solved!\n";
            break;
        }

        int pattern = parse_feedback(input);

        possible_words =
            filter_words(possible_words,
                         best_word.first,
                         pattern);

        if (possible_words.empty())
        {
            std::cout << "No words remain. "
                         "Feedback may be inconsistent.\n";
            break;
        }

        if (possible_words.size() == 1)
        {
            std::cout << "\nAnswer: "
                      << possible_words[0]
                      << std::endl;
            break;
        }
    }
}
