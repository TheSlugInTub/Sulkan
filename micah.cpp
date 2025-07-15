#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

enum Type
{
    Type_Uint8,
    Type_Uint16,
    Type_Uint32,
    Type_Uint64,
    Type_Int8,
    Type_Int16,
    Type_Int32,
    Type_Int64,
    Type_Float32,
    Type_Float64,
    Type_Unserializable, // Pointers
    Type_Custom
};

struct Variable
{
    Type        type;
    std::string typeString;
    std::string identifier;
};

struct Structure
{
    std::vector<Variable> variables;
    std::string           identifier;
};

// Helper function to trim whitespace
std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Helper function to check if a character is valid for identifiers
bool isValidIdentifierChar(char ch)
{
    return std::isalnum(ch) || ch == '_';
}

// Helper function to extract words from a line
std::vector<std::string> extractWords(const std::string& line)
{
    std::vector<std::string> words;
    std::string              word;

    for (size_t i = 0; i < line.length(); i++)
    {
        char ch = line[i];

        if (isValidIdentifierChar(ch))
        {
            word += ch;
        }
        else if (!word.empty())
        {
            words.push_back(word);
            word.clear();
        }
    }

    if (!word.empty())
    {
        words.push_back(word);
    }

    return words;
}

// Helper function to determine variable type enum
Type getTypeFromString(const std::string& typeStr)
{
    if (typeStr == "uint8_t" || typeStr == "unsigned char")
        return Type_Uint8;
    if (typeStr == "uint16_t" || typeStr == "unsigned short")
        return Type_Uint16;
    if (typeStr == "uint32_t" || typeStr == "unsigned int")
        return Type_Uint32;
    if (typeStr == "uint64_t" || typeStr == "unsigned long long")
        return Type_Uint64;
    if (typeStr == "int8_t" || typeStr == "char")
        return Type_Int8;
    if (typeStr == "int16_t" || typeStr == "short")
        return Type_Int16;
    if (typeStr == "int32_t" || typeStr == "int")
        return Type_Int32;
    if (typeStr == "int64_t" || typeStr == "long long")
        return Type_Int64;
    if (typeStr == "float")
        return Type_Float32;
    if (typeStr == "double")
        return Type_Float64;
    if (typeStr.find("*") != std::string::npos)
        return Type_Unserializable;
    return Type_Custom;
}

std::vector<Structure> ParseFile(const char* filePath)
{
    std::ifstream          file(filePath);
    std::vector<Structure> structures;

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filePath
                  << std::endl;
        return structures;
    }

    std::string              line;
    std::vector<std::string> lines;

    // Read all lines
    while (std::getline(file, line)) { lines.push_back(line); }
    file.close();

    Structure currentStructure;
    bool      isParsingStruct = false;
    bool      foundStructKeyword = false;

    for (size_t i = 0; i < lines.size(); i++)
    {
        std::string currentLine = trim(lines[i]);

        // Skip empty lines and comments (except our special comment)
        if (currentLine.empty() ||
            (currentLine.find("//") == 0 &&
             currentLine.find("// COMPONENT") == std::string::npos))
        {
            continue;
        }

        // Check for COMPONENT marker
        if (currentLine.find("// COMPONENT") != std::string::npos)
        {
            // Reset state for new structure
            currentStructure = Structure();
            isParsingStruct = false;
            foundStructKeyword = false;
            continue;
        }

        // Look for struct declaration
        if (!isParsingStruct &&
            currentLine.find("struct") != std::string::npos)
        {
            std::vector<std::string> words =
                extractWords(currentLine);

            // Find "struct" keyword and get the identifier
            for (size_t j = 0; j < words.size(); j++)
            {
                if (words[j] == "struct" && j + 1 < words.size())
                {
                    currentStructure.identifier = words[j + 1];
                    foundStructKeyword = true;
                    break;
                }
            }

            // Check if opening brace is on the same line
            if (currentLine.find("{") != std::string::npos)
            {
                isParsingStruct = true;
            }
            continue;
        }

        // Check for opening brace (if not on same line as struct)
        if (foundStructKeyword && !isParsingStruct &&
            currentLine.find("{") != std::string::npos)
        {
            isParsingStruct = true;
            continue;
        }

        // Check for closing brace
        if (isParsingStruct &&
            currentLine.find("}") != std::string::npos)
        {
            isParsingStruct = false;
            foundStructKeyword = false;

            // Only add structure if it has an identifier
            if (!currentStructure.identifier.empty())
            {
                structures.push_back(currentStructure);
            }
            currentStructure = Structure();
            continue;
        }

        // Parse member variables
        if (isParsingStruct)
        {
            std::vector<std::string> words =
                extractWords(currentLine);

            // Need at least 2 words for type and identifier
            if (words.size() >= 2)
            {
                Variable var;
                var.typeString = words[0];
                var.identifier = words[1];
                var.type = getTypeFromString(var.typeString);

                currentStructure.variables.push_back(var);
            }
        }
    }

    return structures;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "No files provided.\nPlease input a file or "
                     "files as argument(s)."
                  << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        std::cout << "Parsing file: " << argv[i] << std::endl;

        auto structs = ParseFile(argv[i]);

        if (structs.empty())
        {
            std::cout << "No structures found in " << argv[i]
                      << std::endl;
            continue;
        }

        for (const auto& structure : structs)
        {
            std::cout << "Structure: " << structure.identifier
                      << std::endl;
            std::cout << "Variables:" << std::endl;

            for (const auto& var : structure.variables)
            {
                std::cout << "  " << var.typeString << " "
                          << var.identifier << std::endl;
            }
            std::cout << std::endl;
        }
    }

    return 0;
}
