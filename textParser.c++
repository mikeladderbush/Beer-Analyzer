// Creating a reading and parsing method to gather the chat-gpt generated data and fit it into a sqlite database for
// further analysis.

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>

// function declarations
std::string textFileToString(const std::string& filepath);
int textParser(const std::string& filepath);

// vector of keywords that will be looped through and searched for in the parsed text.
std::vector<std::string> keywords = {"Sweet (S)", "Sour", "Salty", "Bitter", "Umami", "fragrantFloral","fruityNonCitrus", "Citrus", "Woody", "Sweet (Sw)", "Minty", "Toasted", "Cereal", "Decayed"};

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
    // Null terminated character is good practice for initializing char.
    int lastFoundAt = 0;
    std::string searchStr = "Sweet (S)";
    char  parameterValue = '\0';
    int posOfSweet = 0;

    // Loops and finds the position of the desired keyword, then loops again starting from that position in order to find the next character that is actually a digit. That digit will for the most part be our assigned value for that parameter.
    for (int i = 0; i < dataToParse.length(); i++){

        posOfSweet = dataToParse.find(searchStr, lastFoundAt);
	size_t digitSearch = posOfSweet + searchStr.length();


	for(size_t j = digitSearch; j < dataToParse.length(); j++){
		if(isdigit(dataToParse[j])){
			parameterValue = dataToParse[j];
			break;
		}
	}
        if(posOfSweet == std::string::npos){
            break;
        }

        lastFoundAt = posOfSweet + searchStr.length();
        printf("%d %c\n",posOfSweet, parameterValue);
    }

    return 0;
}

int main(){
    const std::string& filepath = "hopParams.txt";

    int result = textParser(filepath);

    return result;
}
