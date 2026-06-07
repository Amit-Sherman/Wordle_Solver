#include <fstream>
#include <iostream>
#include <string>
#include <vector>
void print_words(const std::vector<std::pair<std::string, double>> &words)
{
    std::ofstream outputFile("output.txt");
    if (outputFile.is_open())
    {
        for (const auto &word : words)
        {
            outputFile << word.first << ", Entropy: " << word.second << std::endl;
        }
        outputFile.close();
    }
    else
    {
        std::cerr << "Unable to open file for writing." << std::endl;
    }
}
