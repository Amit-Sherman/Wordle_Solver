#include <fstream>
#include <iostream>
#include <vector>
#include <string>
void print_words(const std::vector<std::pair<std::string, int>>& words)
{
    std::ofstream outputFile("output.txt");
    if (outputFile.is_open()){
        for (auto word : words)
        {
            outputFile << word.first  << word.second << std::endl;
        }
        outputFile.close();
    }
    else {
        std::cerr << "Unable to open file for writing." << std::endl;
    }
}
