#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cctype>
#include <unordered_map>

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
    bool        isArray = false;
    int         arrayLength = 0;
};

struct Structure
{
    std::vector<Variable> variables;
    std::string           identifier;
    bool                  isComponent = false;
};

// Helper function to Trim whitespace
std::string Trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Helper function to check if a character is valid for identifiers
bool IsValidIdentifierChar(char ch)
{
    return std::isalnum(ch) || ch == '_';
}

// Helper function to Extract words from a line
std::vector<std::string> ExtractWords(const std::string& line)
{
    std::vector<std::string> words;
    std::string              word;

    for (size_t i = 0; i < line.length(); i++)
    {
        char ch = line[i];

        if (IsValidIdentifierChar(ch))
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

std::pair<std::string, std::pair<bool, int>>
ParseArrayInfo(const std::string& line)
{
    std::string identifier;
    bool        isArray = false;
    int         arrayLength = 0;

    // Find the identifier and check for array brackets
    size_t bracketStart = line.find('[');
    size_t bracketEnd = line.find(']');

    if (bracketStart != std::string::npos &&
        bracketEnd != std::string::npos && bracketEnd > bracketStart)
    {
        isArray = true;

        // Extract identifier (everything before the bracket)
        std::string beforeBracket = line.substr(0, bracketStart);
        std::vector<std::string> words = ExtractWords(beforeBracket);
        if (!words.empty())
        {
            identifier = words.back(); // Last word before bracket is
                                       // the identifier
        }

        // Extract array length from between brackets
        std::string lengthStr = line.substr(
            bracketStart + 1, bracketEnd - bracketStart - 1);
        lengthStr = Trim(lengthStr);

        if (!lengthStr.empty())
        {
            try
            {
                arrayLength = std::stoi(lengthStr);
            }
            catch (const std::exception&)
            {
                // If conversion fails, default to 0
                arrayLength = 0;
            }
        }
    }
    else
    {
        // No array brackets, extract identifier normally
        std::vector<std::string> words = ExtractWords(line);
        if (words.size() >= 2)
        {
            identifier =
                words[1]; // Second word is typically the identifier
        }
    }

    return std::make_pair(identifier,
                          std::make_pair(isArray, arrayLength));
}

// Helper function to determine variable type enum
Type GetTypeFromString(const std::string& typeStr)
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

void ParseFile(const char*             filePath,
               std::vector<Structure>& structures)
{
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filePath
                  << std::endl;
        return;
    }

    std::string              line;
    std::vector<std::string> lines;

    // Read all lines
    while (std::getline(file, line)) { lines.push_back(line); }
    file.close();

    Structure currentStructure;
    bool      isParsingStruct = false;
    bool      foundStructKeyword = false;
    bool      isComponent = false;

    for (size_t i = 0; i < lines.size(); i++)
    {
        std::string currentLine = Trim(lines[i]);

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
            isComponent = true;
            continue;
        }

        // Look for struct declaration
        if (!isParsingStruct &&
            currentLine.find("struct") != std::string::npos)
        {
            std::vector<std::string> words =
                ExtractWords(currentLine);

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
                currentStructure.isComponent = isComponent;
                structures.push_back(currentStructure);
                isComponent = false;
            }
            currentStructure = Structure();
            continue;
        }

        // Parse member variables
        if (isParsingStruct)
        {
            if (currentLine.find("// UNSERIALIZABLE") !=
                std::string::npos)
            {
                continue;
            }

            std::vector<std::string> words =
                ExtractWords(currentLine);

            // Need at least 2 words for type and identifier
            if (words.size() >= 2)
            {
                Variable var;
                var.typeString = words[0];
                var.identifier = words[1];
                var.type = GetTypeFromString(var.typeString);

                auto arrayInfo = ParseArrayInfo(currentLine);
                var.identifier = arrayInfo.first;
                var.isArray = arrayInfo.second.first;
                var.arrayLength = arrayInfo.second.second;

                currentStructure.variables.push_back(var);
            }
        }
    }
}

std::unordered_map<std::string, std::string>
ExtractIgnoredFunctions(const std::string& filename)
{
    std::unordered_map<std::string, std::string> ignoredFuncMap;
    std::ifstream                                inFile(filename);
    if (!inFile)
        return ignoredFuncMap;

    std::vector<std::string> lines;
    std::string              line;
    while (std::getline(inFile, line)) {
        lines.push_back(line);
    }

    for (int i = 0; i < lines.size(); i++)
    {
        if (lines[i].find("// IGNORE") != std::string::npos)
        {
            // Find the function signature (next non-empty line)
            int sigLine = i + 1;
            while (sigLine < lines.size() && lines[sigLine].empty())
                sigLine++;
            if (sigLine >= lines.size())
                continue;

            // Count braces to capture the entire function
            int braceCount = 0;
            int currentLine = sigLine;
            bool foundOpeningBrace = false;

            // Check all lines starting from the signature line
            while (currentLine < lines.size())
            {
                const std::string& currentText = lines[currentLine];
                for (char c : currentText)
                {
                    if (c == '{') {
                        braceCount++;
                        foundOpeningBrace = true;
                    } else if (c == '}') {
                        braceCount--;
                    }
                }

                if (foundOpeningBrace && braceCount == 0)
                    break;

                currentLine++;
            }

            if (braceCount != 0 || currentLine >= lines.size())
                continue;

            // Extract function name from signature line
            std::string sig = lines[sigLine];
            size_t pos = sig.find('(');
            if (pos == std::string::npos)
                continue;
            size_t nameStart = sig.substr(0, pos).find_last_of(" *&");
            nameStart = (nameStart == std::string::npos) ? 0 : nameStart + 1;
            std::string funcName = sig.substr(nameStart, pos - nameStart);

            // Store the entire function (from comment to last brace)
            std::string funcText;
            for (int j = i; j <= currentLine; j++)
            {
                funcText += lines[j] + "\n";
            }

            ignoredFuncMap[funcName] = funcText;
            i = currentLine; // Skip processed lines
        }
    }

    return ignoredFuncMap;
}

void CreateSource(std::vector<Structure>& structures, char** args,
                  int argLength)
{
    auto          ignoredFuncMap = ExtractIgnoredFunctions("src/micah.c");
    std::ofstream sourceFile("src/micah.c");

    sourceFile << "#include \"../micah.h\"\n";

    sourceFile << "\n// micah.c\n";
    sourceFile << "// Procedurally generated source file for the "
                  "Sulkan game engine"
                  "\n// This contains drawer/serializer/deserializer "
                  "functions for all registered components\n";

    for (int i = 0; i < structures.size(); i++)
    {
        Structure& structure = structures[i];

        if (!structure.isComponent)
        {
            continue;
        }

        sourceFile << '\n';

        std::string drawFuncName =
            structure.identifier + "_DrawComponent";
        auto drawIt = ignoredFuncMap.find(drawFuncName);
        if (drawIt != ignoredFuncMap.end())
        {
            sourceFile << drawIt->second << "\n";
        }
        else
        {
            sourceFile << "void " << structure.identifier
                       << "_DrawComponent(" << structure.identifier
                       << "* object, skECSState* state, skEntityID ent)\n{\n";

            sourceFile << "    if (skImGui_CollapsingHeader(\""
                       << structure.identifier << "\"))\n    {\n";

            for (Variable& var : structure.variables)
            {
                if (var.typeString == "int")
                {
                    sourceFile << "        " << "skImGui_InputInt(\""
                               << var.identifier << "\", &object->"
                               << var.identifier << ");\n";
                }
                if (var.typeString == "char" && var.isArray)
                {
                    sourceFile << "        " << "skImGui_InputText(\""
                               << var.identifier << "\", object->"
                               << var.identifier << ", "
                               << var.arrayLength << ", 0);\n";
                }
                if (var.typeString == "mat4")
                {
                    sourceFile << "        "
                               << "skImGui_DragFloat16(\""
                               << var.identifier << "\", object->"
                               << var.identifier << ", 0.1f);\n";
                }
                if (var.typeString == "quat")
                {
                    sourceFile << "        "
                               << "skImGui_DragFloat4(\""
                               << var.identifier << "\", object->"
                               << var.identifier << ", 0.1f);\n";
                }
                if (var.typeString == "vec4")
                {
                    sourceFile << "        "
                               << "skImGui_DragFloat4(\""
                               << var.identifier << "\", object->"
                               << var.identifier << ", 0.1f);\n";
                }
                if (var.typeString == "vec3")
                {
                    sourceFile << "        "
                               << "skImGui_DragFloat3(\""
                               << var.identifier << "\", object->"
                               << var.identifier << ", 0.1f);\n";
                }
                if (var.typeString == "vec2")
                {
                    sourceFile << "        "
                               << "skImGui_DragFloat2(\""
                               << var.identifier << "\", object->"
                               << var.identifier << ", 0.1f);\n";
                }
                if (var.typeString == "float")
                {
                    sourceFile << "        " << "skImGui_DragFloat(\""
                               << var.identifier << "\", &object->"
                               << var.identifier << ", 0.1f);\n";
                }
            }

            sourceFile << "    }\n}\n\n";
        }

        // SERIALIZATION FUNCTIONS

        std::string saveFuncName =
            structure.identifier + "_SaveComponent";
        auto saveIt = ignoredFuncMap.find(saveFuncName);
        if (saveIt != ignoredFuncMap.end())
        {
            sourceFile << saveIt->second << "\n";
        }
        else
        {
            sourceFile << "skJson " << structure.identifier
                       << "_SaveComponent(" << structure.identifier
                       << "* object)\n{\n";

            sourceFile << "    skJson j = skJson_Create();\n\n";

            for (Variable& var : structure.variables)
            {
                if (var.typeString == "int")
                {
                    sourceFile << "    " << "skJson_SaveInt(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "char" && var.isArray)
                {
                    sourceFile << "    " << "skJson_SaveString(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "mat4")
                {
                    sourceFile << "    " << "skJson_SaveFloat16(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "quat")
                {
                    sourceFile << "    " << "skJson_SaveFloat4(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "vec4")
                {
                    sourceFile << "    " << "skJson_SaveFloat4(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "vec3")
                {
                    sourceFile << "    " << "skJson_SaveFloat3(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "vec2")
                {
                    sourceFile << "    " << "skJson_SaveFloat2(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "float")
                {
                    sourceFile << "    " << "skJson_SaveFloat(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "bool")
                {
                    sourceFile << "    " << "skJson_SaveBool(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
            }

            sourceFile << "    return j;\n}\n\n";
        }

        // DESERIALIZATION FUNCTIONS

        std::string loadFuncName =
            structure.identifier + "_LoadComponent";
        auto loadIt = ignoredFuncMap.find(loadFuncName);
        if (loadIt != ignoredFuncMap.end())
        {
            sourceFile << loadIt->second << "\n";
        }
        else
        {
            sourceFile << "void " << structure.identifier
                       << "_LoadComponent(" << structure.identifier
                       << "* object, skJson j)\n{\n";

            for (Variable& var : structure.variables)
            {
                if (var.typeString == "int")
                {
                    sourceFile << "    " << "skJson_LoadInt(j, \""
                               << var.identifier << "\", &object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "char" && var.isArray)
                {
                    sourceFile << "    " << "skJson_LoadString(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "mat4")
                {
                    sourceFile << "    " << "skJson_LoadFloat16(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "quat")
                {
                    sourceFile << "    " << "skJson_LoadFloat4(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "vec4")
                {
                    sourceFile << "    " << "skJson_LoadFloat4(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "vec3")
                {
                    sourceFile << "    " << "skJson_LoadFloat3(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "vec2")
                {
                    sourceFile << "    " << "skJson_LoadFloat2(j, \""
                               << var.identifier << "\", object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "float")
                {
                    sourceFile << "    " << "skJson_LoadFloat(j, \""
                               << var.identifier << "\", &object->"
                               << var.identifier << ");\n";
                }
                else if (var.typeString == "bool")
                {
                    sourceFile << "    " << "skJson_LoadBool(j, \""
                               << var.identifier << "\", &object->"
                               << var.identifier << ");\n";
                }
            }

            sourceFile << "}\n";
        }
    }

    sourceFile << "\n";

    sourceFile << "void Micah_DrawAllComponents(skECSState* state, "
                  "skEntityID ent)\n{\n";

    for (int i = 0; i < structures.size(); i++)
    {
        Structure& structure = structures[i];

        if (!structure.isComponent)
        {
            continue;
        }

        std::string objIdent = structure.identifier + "Obj";
        sourceFile << "    " << structure.identifier << "* "
                   << structure.identifier
                   << "Obj = SK_ECS_GET(state->scene, ent, "
                   << structure.identifier << ");\n\n";
        sourceFile << "    if (" << objIdent << " != NULL)\n";
        sourceFile << "    {\n";
        sourceFile << "        " << structure.identifier
                   << "_DrawComponent(" << objIdent << ", state, ent);\n";
        sourceFile << "    }\n\n";
    }

    sourceFile << "}\n\n";

    sourceFile << "skJson Micah_SaveAllComponents(skECSState* "
                  "state)\n{\n";

    sourceFile << "    skJson j = skJson_Create();\n\n";
    sourceFile
        << "    for (int i = 0; i < "
           "skECS_EntityCount(state->scene); i++)\n    {\n      "
           "  skEntityID ent = "
           "skECS_GetEntityAtIndex(state->scene, "
           "i);\n        skJson entJson = skJson_Create();\n\n   "
           "    "
           " if (!skECS_IsEntityValid(ent))\n        {\n         "
           " "
           "  continue;\n        }\n\n";

    for (int i = 0; i < structures.size(); i++)
    {
        Structure& structure = structures[i];

        if (!structure.isComponent)
        {
            continue;
        }

        std::string objIdent = structure.identifier + "Obj";
        sourceFile << "        " << structure.identifier << "* "
                   << structure.identifier
                   << "Obj = SK_ECS_GET(state->scene, ent, "
                   << structure.identifier << ");\n\n";
        sourceFile << "        if (" << objIdent << " != NULL)\n";
        sourceFile << "        {\n";
        sourceFile << "            skJson compJson = "
                   << structure.identifier << "_SaveComponent("
                   << objIdent << ");\n";
        sourceFile << "            skJson_SaveString(compJson, "
                      "\"componentType\", \""
                   << structure.identifier << "\");\n";
        sourceFile << "            skJson_PushBack(entJson, "
                      "compJson);\n";
        sourceFile << "            skJson_Destroy(compJson);\n";
        sourceFile << "        }\n\n";
    }

    sourceFile << "        skJson_PushBack(j, entJson);\n        "
                  "skJson_Destroy(entJson);\n    }\n\n   "
                  " return j;\n}\n";

    sourceFile << "\n";

    sourceFile << "\n";

    sourceFile << "void Micah_LoadAllComponents(skECSState* state, "
                  "skJson j)\n{\n";
    sourceFile << "    int entityCount = skJson_GetArraySize(j);\n\n";
    sourceFile
        << "    for (int i = 0; i < entityCount; i++)\n    {\n";
    sourceFile << "        skJson entJson = "
                  "skJson_GetArrayElement(j, i);\n";
    sourceFile << "        skEntityID ent = "
                  "skECS_AddEntity(state->scene);\n\n";
    sourceFile << "        int componentCount = "
                  "skJson_GetArraySize(entJson);\n\n";
    sourceFile << "        for (int compIndex = 0; compIndex < "
                  "componentCount; compIndex++)\n        {\n";
    sourceFile << "            skJson compJson = "
                  "skJson_GetArrayElement(entJson, compIndex);\n\n";
    sourceFile << "            char componentType[256];\n";
    sourceFile << "            skJson_LoadString(compJson, "
                  "\"componentType\", "
                  "componentType);\n\n";
    sourceFile << "            // Check component type and "
                  "assign/load accordingly\n";
    sourceFile << "            if (false) {} // Dummy condition for "
                  "cleaner generated code\n";

    for (int i = 0; i < structures.size(); i++)
    {
        Structure& structure = structures[i];

        if (!structure.isComponent)
        {
            continue;
        }

        sourceFile << "            else if (strcmp(componentType, \""
                   << structure.identifier << "\") == 0)\n";
        sourceFile << "            {\n";
        sourceFile << "                " << structure.identifier
                   << "* comp = SK_ECS_ASSIGN(state->scene, ent, "
                   << structure.identifier << ");\n";
        sourceFile << "                " << structure.identifier
                   << "_LoadComponent(comp, compJson);\n";
        sourceFile << "            }\n";
    }

    sourceFile << "        }\n";
    sourceFile << "    }\n";
    sourceFile << "}\n\n";

    sourceFile
        << "void Micah_ComponentAddMenu(skECSState* state, "
           "skEntityID ent)\n{\n"
           "    if (skImGui_BeginPopupContextWindow())\n    {\n";

    for (int i = 0; i < structures.size(); i++)
    {
        Structure& structure = structures[i];

        if (!structure.isComponent)
        {
            continue;
        }

        sourceFile << "        if (skImGui_MenuItem(\""
                   << structure.identifier << "\"))\n        {\n";
        sourceFile << "            SK_ECS_ASSIGN(state->scene, ent, "
                   << structure.identifier << ");\n";
        sourceFile << "        }\n";
    }

    sourceFile << "\n        skImGui_EndPopup();\n    }\n}\n";

    sourceFile.close();
}

void CreateHeader(std::vector<Structure>& structures, char** args,
                  int argLength)
{
    std::ofstream sourceFile("micah.h");

    sourceFile << "#pragma once\n\n";

    for (int j = 1; j < argLength; j++)
    {
        sourceFile << "#include \"" << args[j] << "\"\n";
    }

    sourceFile << "#include \"include/sulkan/imgui_layer.h\"\n";
    sourceFile << "#include \"include/sulkan/state.h\"\n";
    sourceFile << "#include \"include/sulkan/json_api.h\"\n";

    sourceFile << "\n// micah.h\n";
    sourceFile << "// Procedurally generated header file for the "
                  "Sulkan game engine"
                  "\n// This contains drawer/serializer/deserializer "
                  "function declarations for all registered components\n";

    for (int i = 0; i < structures.size(); i++)
    {
        Structure& structure = structures[i];

        if (!structure.isComponent)
        {
            continue;
        }

        sourceFile << '\n';

        std::string drawFuncName =
            structure.identifier + "_DrawComponent";
        
        sourceFile << "void " << structure.identifier
                   << "_DrawComponent(" << structure.identifier
                   << "* object, skECSState* state, skEntityID ent);\n\n";

        // SERIALIZATION FUNCTIONS

        std::string saveFuncName =
            structure.identifier + "_SaveComponent";
        sourceFile << "skJson " << structure.identifier
                   << "_SaveComponent(" << structure.identifier
                   << "* object);\n\n";

        // DESERIALIZATION FUNCTIONS

        std::string loadFuncName =
            structure.identifier + "_LoadComponent";
        sourceFile << "void " << structure.identifier
                   << "_LoadComponent(" << structure.identifier
                   << "* object, skJson j);\n\n";
    }

    sourceFile << "void Micah_DrawAllComponents(skECSState* state, "
                  "skEntityID ent);\n\n";

    sourceFile << "skJson Micah_SaveAllComponents(skECSState* "
                  "state);\n\n";

    sourceFile << "void Micah_LoadAllComponents(skECSState* state, "
                  "skJson j);\n\n";
    sourceFile
        << "void Micah_ComponentAddMenu(skECSState* state, "
           "skEntityID ent);\n\n";

    sourceFile.close();
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

    std::vector<Structure> structs;

    for (int i = 1; i < argc; i++)
    {
        std::cout << "Parsing file: " << argv[i] << std::endl;

        ParseFile(argv[i], structs);
    }
    if (structs.empty())
    {
        std::cout << "No structures found" << std::endl;
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
            std::cout << "  " << var.arrayLength << " " << '\n';
        }
        std::cout << std::endl;
    }

    CreateSource(structs, argv, argc);
    CreateHeader(structs, argv, argc);

    return 0;
}
