// Creating a reading and parsing method to gather the chat-gpt generated data and fit it into a sqlite database for
// further analysis.

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>

// function declarations
std::string textFileToString(const std::string& filepath);
int fileParser(const std::string& filepath);


struct Hop{
    std::string name;

    struct Flavor{
        int sweet;
        int sour;
        int salty;
        int bitter;
        int umami;
    };

    struct Smell{

        int fragrantFloral;
        int fruityNonCitrus;
        int citrus;
        int woody;
        int chemical;
        int sweetSmell;
        int minty;
        int toasted;
        int cereal;
        int decayed;

    };

};


// Providing a filepath 
std::string textFileToString(const std::string& filepath){

    // Opens the file
    std::fstream file(filepath);

    // Simple error-checking to make sure the file has opened.
    if(!file.is_open()){
        std::cerr << "Failed to open the file" << filepath << std::endl;
    }

    // using a stringstream buffer I move the file contents into the buffer and return it.
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// take the filepath string as a parameter
int textParser(const std::string& filepath){

    // stringify the data inside the text file from the path
    std::string dataToParse = textFileToString(filepath);

    // define the first instance of the found variable, in this case its 0 because we havent looked yet.
    int lastFoundAt = 0;

    // loop through the length of the data finding "Sweet" each time and then marking the position as the last occurance
    // in order to not repeat occurances. If no other sweet can be found the loop will break.
    // This is for testing purposes so I can eventually create a system for parsing.
    for (int i = 0; i < dataToParse.length(); i++){
        int posOfSweet = dataToParse.find("Sweet", lastFoundAt);

        if(posOfSweet == std::string::npos){
            break;
        }

        lastFoundAt = posOfSweet + 1;
        printf("%d\n",posOfSweet);
    }

    return 0;
}

int main(){
    const std::string& filepath = "C:/Users/Michael Ladderbush/Desktop/C++ Projects/Beer Analyzer/hopParams.txt";

    int result = textParser(filepath);

    return result;
}