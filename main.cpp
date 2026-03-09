#include <iostream>
#include "Helper.cpp"

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
