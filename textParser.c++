// Creating a reading and parsing method to gather the chat-gpt generated data and fit it into a sqlite database for
// further analysis.

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <map>

// function declarations
std::string textFileToString(const std::string& filepath);
int textParser(const std::string& filepath);

// vector of keywords that will be looped through and searched for in the parsed text.
std::vector<std::string> keywords = {"Sweet (S)", "Sour", "Salty", "Bitter", 
                            "Umami", "fragrantFloral","fruityNonCitrus", "Citrus", 
                            "Woody", "Sweet (Sw)", "Minty", "Toasted", "Cereal", "Decayed"};

struct Hop{

    std::string name;
    std::string type;

    struct Flavor {

        int sweet;
        int sour;
        int salty;
        int bitter;
        int umami;

    } flavor;

    struct Smell {

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

    } smell;

};

// initialize a hop and a string:int map so that we can map the hop member variables to the keywords. 
// Then we can parse for the keywords and their values before saving the values into the hop structs.
Hop hop;

std::map<std::string,int*> keywordToStructMember = {{"Sweet (S)", &hop.flavor.sweet}, {"Sour", &hop.flavor.sour},
                                    {"Salty", &hop.flavor.salty}, {"Bitter", &hop.flavor.bitter}, {"Umami", &hop.flavor.umami},
                                    {"fragrantFloral", &hop.smell.fragrantFloral}, {"fruityNonCitrus", &hop.smell.fruityNonCitrus},
                                    {"Citrus", &hop.smell.citrus},{"Woody", &hop.smell.woody},{"Sweet (Sw)", &hop.smell.sweetSmell},
                                    {"Minty", &hop.smell.minty},{"Toasted", &hop.smell.toasted},{"Cereal", &hop.smell.cereal},
                                    {"Decayed", &hop.smell.decayed}};



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
    char parameterValue = '\0';
    size_t lastPos = 0;
std::regex regexStr("\\W\\W\\s\\w+\\s");
std::smatch regexMatch;

    // Loops and finds the position of the desired keyword, then loops again starting from that position in order to find the next character that is actually a digit. That digit will for the most part be our assigned value for that parameter.
while(std::regex_search(dataToParse.begin() + lastPos, dataToParse.end(), regexMatch, regexStr)){
	
	if(!regexMatch.ready()){
		break;
	}

	std::string hopTitle = regexMatch[1].matched ? regexMatch[1].str() : regexMatch[2].str();
	hop.name = hopTitle;
	lastPos += regexMatch.position() + regexMatch.length();

    for (const std::string& word : keywords){

        for (int i = 0; i < dataToParse.length(); i++){

            int posOfWord = dataToParse.find(word, lastFoundAt);

	        if (posOfWord == std::string::npos){

		        break;
	        }

	        size_t digitSearch = posOfWord + word.length();
	        parameterValue = '\0';


	        for(size_t j = digitSearch; j < dataToParse.length(); j++){

		        if(isdigit(dataToParse[j])){

			        parameterValue = dataToParse[j];
			        break;

		        }

	        }
		//Checks if the values for both the parameter and keyword are valid and then assigns the value in the map. This will later be used to create hop structures which will be moved into the database. Will be implemented once the name and type values are also completed.		
	        if(parameterValue != '\0'){

		        if(keywordToStructMember.find(word) != keywordToStructMember.end()){

			    *keywordToStructMember[word] = parameterValue - '0';

		        }

	        }

	        // position doesnt matter but need the title of the parameter variable and then to store that value into the struct.
            lastFoundAt = posOfWord + word.length();
		
        }

    }

	std::cout << "test" << "\n";
	std::cout << hop.flavor.sweet << "\n";
}

return 0;	

}

int main(){

    const std::string& filepath = "hopParams.txt";

    int result = textParser(filepath);

    return result;

}
