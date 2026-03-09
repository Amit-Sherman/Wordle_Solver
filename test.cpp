#include <fstream>
#include <iostream>
#include <vector>
#include <string>
void print_words(const std::vector<std::string>& words)
{
    std::ofstream outputFile("output.txt");
    if (outputFile.is_open()){
        for (const std::string& word : words)
        {
            outputFile << word << std::endl;
        }
        outputFile.close();
    }
    else {
        std::cerr << "Unable to open file for writing." << std::endl;
    }
}