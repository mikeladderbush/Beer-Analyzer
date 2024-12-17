// Creating a reading and parsing method to gather the chat-gpt generated data and fit it into a sqlite database for
// further analysis.

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <map>
#include "sqlite3.h"

// function declarations
std::string textFileToString(const std::string &filepath);
int textParser(const std::string &filepath);

// vector of keywords that will be looped through and searched for in the parsed text.
std::vector<std::string> keywords = {"Sweet (S)", "Sour", "Salty", "Bitter",
                                     "Umami", "fragrantFloral", "fruityNonCitrus", "Citrus",
                                     "Woody", "Sweet (Sw)", "Minty", "Toasted", "Cereal", "Decayed"};

struct Hop
{

    std::string name;
    std::string type;

    struct Flavor
    {

        int sweet;
        int sour;
        int salty;
        int bitter;
        int umami;

    } flavor;

    struct Smell
    {

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

std::map<std::string, int *> keywordToStructMember = {{"Sweet (S)", &hop.flavor.sweet}, {"Sour", &hop.flavor.sour}, {"Salty", &hop.flavor.salty}, {"Bitter", &hop.flavor.bitter}, {"Umami", &hop.flavor.umami}, {"fragrantFloral", &hop.smell.fragrantFloral}, {"fruityNonCitrus", &hop.smell.fruityNonCitrus}, {"Citrus", &hop.smell.citrus}, {"Woody", &hop.smell.woody}, {"Sweet (Sw)", &hop.smell.sweetSmell}, {"Minty", &hop.smell.minty}, {"Toasted", &hop.smell.toasted}, {"Cereal", &hop.smell.cereal}, {"Decayed", &hop.smell.decayed}};

// Providing a filepath
std::string textFileToString(const std::string &filepath)
{

	// Creating a sqlite3 object which is the connection to our database
	sqlite3 *db;
	int dbOpenErr = sqlite3_open("MasterBeerParams.db", &db);

	// error checking
	if (dbOpenErr){
		std::cerr << "Issue opening the database" << sqlite3_errmsg(db) << std::endl;
	}

	// Next I make a sql statement which will be executed using the sqlite3_exec function. The function accepts the database connection as well as a pointer to the statement C string
	const char *sql = "CREATE TABLE IF NOT EXISTS hopData (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL, Type TEXT NOT NULL, Sweet INTEGER, Bitter INTEGER, Sour INTEGER, Salty INTEGER, Umami INTEGER, Floral INTEGER, Fruity INTEGER, Citrus INTEGER, Woody INTEGER, Chemical INTEGER, SweetSmell INTEGER, Minty INTEGER, Toasted INTEGER, Popcorn INTEGER, Decayed INTEGER);"; 
	dbOpenErr = sqlite3_exec(db, sql, 0, 0, 0);

	if (dbOpenErr != SQLITE_OK){
		std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
	}

	// use INSERT INTO db (param) VALUES ('xxx'); to start inserting tomorrow, stopping for now...headache


    // Opens the file
    std::fstream file(filepath);

    // Simple error-checking to make sure the file has opened.
    if (!file.is_open())
    {

        std::cerr << "Failed to open the file" << filepath << std::endl;
        return "Error formatting file to string";
    }

    // using a stringstream buffer I move the file contents into the buffer and return it.
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// take the filepath string as a parameter
int textParser(const std::string &filepath)
{

    // stringify the data inside the text file from the path
    std::string dataToParse = textFileToString(filepath);

    // define the first instance of the found variable, in this case its 0 because we havent looked yet.
    // Null terminated character is good practice for initializing char.
    int lastFoundAt = 0;
    char parameterValue = '\0';
    std::regex regexStr(R"(#### \d+\.\s+\*\*(.*?)\*\*)");
    std::smatch regexMatch;

    // Updated to use a regex search that will look for patterns that represent the titles of our hops.
    // If a hop title is found then the dataToParse becomes all following data in order to ignore anything
    // before what was already found and stored. After the name is found then we iterate through
    // our keywords and find all of their values by locating the first digit in the new dataToParse, then
    // saving them into the hop structure.
    while (std::regex_search(dataToParse, regexMatch, regexStr))
    {

        if (!regexMatch.ready())
        {
            break;
        }

        std::string hopTitle = regexMatch[1].matched ? regexMatch[1].str() : regexMatch[2].str();
        hop.name = hopTitle;

        dataToParse = regexMatch.suffix().str();

        for (const std::string &word : keywords)
        {

            // position is found and then the new starting place for the loop to look is at the end
            // of that word.
            int posOfWord = dataToParse.find(word, lastFoundAt);
            lastFoundAt = posOfWord + word.length();

            if (posOfWord == std::string::npos)
            {

                break;
            }

            // Reset param to ensure correct new value.
            parameterValue = '\0';

            // new data substring is searched for the first digit and the value at that location is
            // given to the parameterValue variable.
            int digitSearch = dataToParse.find_first_of("0123456789", lastFoundAt);

            if (digitSearch != std::string::npos)
            {

                parameterValue = dataToParse[digitSearch];

                // Checks if the values for both the parameter and keyword are valid and then assigns the value in the map. This will later be used to create hop structures which will be moved into the database. Will be implemented once the name and type values are also completed.
                if (parameterValue != '\0')
                {

                    if (keywordToStructMember.find(word) != keywordToStructMember.end())
                    {

                        *keywordToStructMember[word] = parameterValue - '0';
                    }
                }
            }
            else
            {

                continue;
            }
        }
        std::cout << hop.name << "\n";
        std::cout << hop.flavor.sweet << "\n";
    }

    return 0;
}

int main()
{

    const std::string &filepath = "hopParams.txt";

    int result = textParser(filepath);

    return result;
}
