#include "Helper.cpp"
#include "test.cpp"
#include <numeric>
#include <random>

#define NUM_OF_GAMES 100

int main()
{
    // load words dataset
    std::cout << "Loading dataset . . ." << std::endl;
    auto words_dataset = load_data("words_file.txt");
    std::cout << "Finished loading dataset" << std::endl;
    // pick a random target
    std::random_device rd;
    std::mt19937 gen(rd());
    int size = words_dataset.size();
    std::uniform_int_distribution<> distrib(0, size - 1);

    // calculate the best first word for all games, because it is always the same
    std::cout << "Finding first best word to guess . . ." << std::endl;
    auto first_best_word = best_words_to_guess(words_dataset, words_dataset)[0];
    std::vector<int> guesses;

    for (int i = 0; i < NUM_OF_GAMES; i++)
    {
        auto possible_words = words_dataset;
        std::string target = possible_words[distrib(gen)];
        std::cout << "Target is: " << target << std::endl;
        int n_guesses = 0;
        while (possible_words.size() > 1)
        {
            std::pair<std::string, double> best_word;
            if (n_guesses == 0)
            {
                best_word = first_best_word;
            }
            else
                best_word = best_words_to_guess(words_dataset, possible_words)[0];
            std::cout << "Guessing the word: " << best_word.first
                      << ", with average information gain of: " << best_word.second << " bits" << std::endl;
            int feedback_pattern = feedback(best_word.first, target);
            possible_words = filter_words(possible_words, best_word.first, feedback_pattern);
            n_guesses++;
        }
        guesses.push_back(n_guesses);
    }
    double sum = std::accumulate(guesses.begin(), guesses.end(), 0.0);
    std::cout << "Average number of guesses is: " << sum / NUM_OF_GAMES << std::endl;
    auto minmax_it = std::minmax_element(guesses.begin(), guesses.end());
    int min_val = *minmax_it.first;
    int max_val = *minmax_it.second;
    std::cout << "Min number of gusses is: " << min_val << std::endl;
    std::cout << "Max number of gusses is: " << max_val << std::endl;
    return 0;
}
