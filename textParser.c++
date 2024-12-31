// Creating a reading and parsing method to gather the chat-gpt generated data and fit it into a sqlite database for
// further analysis.

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <map>
#include <filesystem>
#include "sqlite3.h"

// function declarations
std::string textFileToString(const std::string &filepath);
int textParser(const std::string &filepath);
std::string getBaseName(const std::string &filepath);

// vector of keywords that will be looped through and searched for in the parsed text.
std::vector<std::string> keywords = {
    "Sweet (S)", "Sour", "Salty", "Bitter", "Umami",
    "Fragrant/Floral (Fg)", "Fruity (non-citrus) (Fr)", "Citrus (Ci)",
    "Woody/Resinous (Wo)", "Chemical (Ch)", "Sweet (Sw)",
    "Minty/Herbal/Pungent (Mi)", "Toasted/Nutty (To)", "Popcorn/Cereal-like (Po)",
    "Decayed/Rancid (De)"};

struct Ingredient
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

// initialize a ingredient and a string:int map so that we can map the ingredient member variables to the keywords.
// Then we can parse for the keywords and their values before saving the values into the ingredient structs.
Ingredient ingredient;

std::map<std::string, int *> keywordToStructMember = {
    {"Sweet (S)", &ingredient.flavor.sweet}, {"Sour", &ingredient.flavor.sour}, {"Salty", &ingredient.flavor.salty}, {"Bitter", &ingredient.flavor.bitter}, {"Umami", &ingredient.flavor.umami}, {"Fragrant/Floral (Fg)", &ingredient.smell.fragrantFloral}, {"Fruity (non-citrus) (Fr)", &ingredient.smell.fruityNonCitrus}, {"Citrus (Ci)", &ingredient.smell.citrus}, {"Woody/Resinous (Wo)", &ingredient.smell.woody}, {"Chemical (Ch)", &ingredient.smell.chemical}, {"Sweet (Sw)", &ingredient.smell.sweetSmell}, {"Minty/Herbal/Pungent (Mi)", &ingredient.smell.minty}, {"Toasted/Nutty (To)", &ingredient.smell.toasted}, {"Popcorn/Cereal-like (Po)", &ingredient.smell.cereal}, {"Decayed/Rancid (De)", &ingredient.smell.decayed}};

// Providing a filepath
std::string textFileToString(const std::string &filepath)
{

    // Opens the file
    std::fstream file(filepath);

    // Simple error-checking to make sure the file has opened.
    if (!file.is_open())
    {

        std::cerr << "Failed to open the file" << filepath << std::endl;
        return "";
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
    if (dataToParse.empty())
    {
        return 1;
    }

    // Creating a sqlite3 object which is the connection to our database
    sqlite3 *db;
    int dbOpenErr = sqlite3_open("MasterBeerParams.db", &db);

    // error checking
    if (dbOpenErr)
    {
        std::cerr << "Issue opening the database" << sqlite3_errmsg(db) << std::endl;
    }

    std::string baseName = getBaseName(filepath);
    std::string tableName = baseName + "_DataTable";

    // Next I make a sql statement which will be executed using the sqlite3_exec function. The function accepts the database connection as well as a pointer to the statement C string
    std::string sql = "CREATE TABLE IF NOT EXISTS " + tableName + " (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL, Type TEXT NOT NULL, Sweet INTEGER, Bitter INTEGER, Sour INTEGER, Salty INTEGER, Umami INTEGER, Floral INTEGER, Fruity INTEGER, Citrus INTEGER, Woody INTEGER, Chemical INTEGER, SweetSmell INTEGER, Minty INTEGER, Toasted INTEGER, Cereal INTEGER, Decayed INTEGER, UNIQUE(NAME, TYPE));";
    dbOpenErr = sqlite3_exec(db, sql.c_str(), 0, 0, 0);

    if (dbOpenErr != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    // define the first instance of the found variable, in this case its 0 because we havent looked yet.
    // Null terminated character is good practice for initializing char.
    char parameterValue = '\0';
    std::regex regexStr(R"(#### \d+\.\s+\*\*(.*?)\*\*)");
    std::smatch regexMatch;

    // Updated to use a regex search that will look for patterns that represent the titles of our ingredients.
    // If a ingredient title is found then the dataToParse becomes all following data in order to ignore anything
    // before what was already found and stored. After the name is found then we iterate through
    // our keywords and find all of their values by locating the first digit in the new dataToParse, then
    // saving them into the ingredient structure.
    while (std::regex_search(dataToParse, regexMatch, regexStr))
    {
        int lastFoundAt = 0;

        if (!regexMatch.ready())
        {
            break;
        }

        std::string ingredientTitle = regexMatch[1].matched ? regexMatch[1].str() : regexMatch[2].str();
        ingredient.name = ingredientTitle;

        dataToParse = regexMatch.suffix().str();

        for (const std::string &word : keywords)
        {

            // position is found and then the new starting place for the loop to look is at the end
            // of that word.
            int posOfWord = dataToParse.find(word, lastFoundAt);

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

                // Checks if the values for both the parameter and keyword are valid and then assigns the value in the map. This will later be used to create ingredient structures which will be moved into the database. Will be implemented once the name and type values are also completed.
                if (parameterValue != '\0')
                {

                    if (keywordToStructMember.find(word) != keywordToStructMember.end())
                    {

                        *keywordToStructMember[word] = parameterValue - '0';
                    }
                }
                lastFoundAt = digitSearch + 1;
            }
            else
            {

                continue;
            }
        }

        // Created sql statement string which will be converted to a C string before being sent to the database.
        std::string sql = "INSERT INTO " + tableName + " (Name, Type, Sweet, Sour, Salty, Bitter, Umami, Floral, Fruity, Citrus, Woody, Chemical, SweetSmell, Minty, Toasted, Cereal, Decayed) "
                                                       "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

        sqlite3_stmt *stmt;

        // Statement is prepared by establishing the database, the statement, the number of bytes, the statement handle and an unsused portion
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "Statement incorrectly created: " << sqlite3_errmsg(db) << std::endl;
            return 1;
        }

        // After preparing the statement we can bind the struct values before inserting.
        sqlite3_bind_text(stmt, 1, ingredient.name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, ingredient.type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, ingredient.flavor.sweet);
        sqlite3_bind_int(stmt, 4, ingredient.flavor.sour);
        sqlite3_bind_int(stmt, 5, ingredient.flavor.salty);
        sqlite3_bind_int(stmt, 6, ingredient.flavor.bitter);
        sqlite3_bind_int(stmt, 7, ingredient.flavor.umami);
        sqlite3_bind_int(stmt, 8, ingredient.smell.fragrantFloral);
        sqlite3_bind_int(stmt, 9, ingredient.smell.fruityNonCitrus);
        sqlite3_bind_int(stmt, 10, ingredient.smell.citrus);
        sqlite3_bind_int(stmt, 11, ingredient.smell.woody);
        sqlite3_bind_int(stmt, 12, ingredient.smell.chemical);
        sqlite3_bind_int(stmt, 13, ingredient.smell.sweetSmell);
        sqlite3_bind_int(stmt, 14, ingredient.smell.minty);
        sqlite3_bind_int(stmt, 15, ingredient.smell.toasted);
        sqlite3_bind_int(stmt, 16, ingredient.smell.cereal);
        sqlite3_bind_int(stmt, 17, ingredient.smell.decayed);

        // Once the values are bound to the statement the query can be executed
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        }
        else
        {
            std::cout << "ingredient struct inserted successfully" << std::endl;
        }

        // Finalizing sql statement to close out.
        sqlite3_finalize(stmt);

        std::cout << ingredient.name << "\n";
        std::cout << ingredient.flavor.sweet << "\n";
    }

    return 0;
}

std::string getBaseName(const std::string &filepath)
{
    std::filesystem::path path(filepath);
    return path.stem().string();
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        std::cerr << "Too many arguments, please only provide the filepath to be parsed" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];

    int result = textParser(filepath);

    return result;
}
