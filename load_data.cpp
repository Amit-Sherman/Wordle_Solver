#include <iostream>
#include <string>
#include <vector>
#include <fstream>
std::vector<std::string> load_data(std::string filename)
{
    std::vector<std::string> data;
    // Implementation for loading data from file
    std::ifstream wordsFile(filename);
    if (!wordsFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return std::vector<std::string>();
    }
    std::string word;
    while (wordsFile >> word) {
        data.push_back(word);
    }
    return data;
}