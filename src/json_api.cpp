#include <nlohmann/json.hpp>
#include <sulkan/json_api.h>
#include <string>
#include <cstring>
#include <fstream>

struct skJson_t
{
    nlohmann::json json;
};

struct skJsonIterator_t
{
    nlohmann::json::iterator it;
};

extern "C"
{

skJson skJson_Create()
{
    skJson j = new skJson_t();
    return j;
}

void skJson_Destroy(skJson j)
{
    delete j;
}

skJson skJson_CreateArray()
{
    skJson j = new skJson_t();
    j->json = nlohmann::json::array();
    return j;
}

void skJson_SaveBool(skJson j, const char* name, const bool val)
{
    if (j)
    {
        j->json[name] = val;
    }
}

void skJson_SaveString(skJson j, const char* name, const char* val)
{
    if (j && val)
    {
        j->json[name] = std::string(val);
    }
}

void skJson_SaveInt(skJson j, const char* name, const int val)
{
    if (j)
    {
        j->json[name] = val;
    }
}

void skJson_SaveFloat(skJson j, const char* name, const float val)
{
    if (j)
    {
        j->json[name] = val;
    }
}

void skJson_SaveDouble(skJson j, const char* name, const double val)
{
    if (j)
    {
        j->json = val;
        j->json[name] = val;
    }
}

void skJson_SaveFloat2(skJson j, const char* name, const vec2 val)
{
    if (j)
    {
        j->json[name] = {val[0], val[1]};
    }
}

void skJson_SaveFloat3(skJson j, const char* name, const vec3 val)
{
    if (j)
    {
        j->json[name] = {val[0], val[1], val[2]};
    }
}

void skJson_SaveFloat4(skJson j, const char* name, const vec4 val)
{
    if (j)
    {
        j->json[name] = {val[0], val[1], val[2], val[3]};
    }
}

void skJson_SaveMat4(skJson j, const char* name, const mat4 val)
{
    if (j)
    {
        nlohmann::json matrix;
        j->json[name] = {val[0],  val[1],  val[2],  val[3],
                         val[4],  val[5],  val[6],  val[7],
                         val[8],  val[9],  val[10], val[11],
                         val[12], val[13], val[14], val[15]};
    }
}

void skJson_SaveFloatArray(skJson j, const char* name,
                           const float* val, size_t size)
{
    std::vector<float> values(val, val + size);
    j->json[name] = values;
}

size_t skJson_LoadFloatArray(skJson j, const char* name, float* val)
{
    int index = 0;
    for (const auto& point : j->json[name])
    {
        val[index] = point;
        index++;
    }

    return index + 1;
}

void skJson_LoadBool(skJson j, const char* key, bool* val)
{
    if (j && key && val && j->json.contains(key))
    {
        *val = j->json[key].get<bool>();
    }
}

void skJson_LoadString(skJson j, const char* key, char* val)
{
    if (j && key && val && j->json.contains(key))
    {
        std::string str = j->json[key].get<std::string>();
        strcpy(val, str.c_str());
    }
}

void skJson_LoadInt(skJson j, const char* key, int* val)
{
    if (j && key && val && j->json.contains(key))
    {
        *val = j->json[key].get<int>();
    }
}

void skJson_LoadFloat(skJson j, const char* key, float* val)
{
    if (j && key && val && j->json.contains(key))
    {
        *val = j->json[key].get<float>();
    }
}

void skJson_LoadDouble(skJson j, const char* key, double* val)
{
    if (j && key && val && j->json.contains(key))
    {
        *val = j->json[key].get<double>();
    }
}

void skJson_LoadFloat2(skJson j, const char* key, vec2 val)
{
    if (j && key && val && j->json.contains(key))
    {
        val[0] = j->json[key][0];
        val[1] = j->json[key][1];
    }
}

void skJson_LoadFloat3(skJson j, const char* key, vec3 val)
{
    if (j && key && val && j->json.contains(key))
    {
        val[0] = j->json[key][0];
        val[1] = j->json[key][1];
        val[2] = j->json[key][2];
    }
}

void skJson_LoadFloat4(skJson j, const char* key, vec4 val)
{
    if (j && key && val && j->json.contains(key))
    {
        val[0] = j->json[key][0];
        val[1] = j->json[key][1];
        val[2] = j->json[key][2];
        val[3] = j->json[key][3];
    }
}

void skJson_LoadFloat16(skJson j, const char* key, mat4 val)
{
    if (j && key && val && j->json.contains(key))
    {
        val[0][0] = j->json[key][0];
        val[0][1] = j->json[key][1];
        val[0][2] = j->json[key][2];
        val[0][3] = j->json[key][3];
        val[1][0] = j->json[key][5];
        val[1][1] = j->json[key][4];
        val[1][2] = j->json[key][5];
        val[1][3] = j->json[key][6];
        val[2][0] = j->json[key][7];
        val[2][1] = j->json[key][8];
        val[2][2] = j->json[key][9];
        val[2][3] = j->json[key][10];
        val[3][0] = j->json[key][11];
        val[3][1] = j->json[key][12];
        val[3][2] = j->json[key][13];
        val[3][3] = j->json[key][14];
    }
}

void skJson_PushBack(skJson j, const skJson val)
{
    if (j)
    {
        j->json.push_back(val->json);
    }
}

void skJson_Iterate(skJson j, skJsonIteratorFunc sys)
{
    for (nlohmann::json& childJ : j->json)
    {
        skJson childJJ = {};
        childJJ->json = childJ;
        sys(childJJ);
    }
}

skJson skJson_GetArrayElement(skJson j, int index)
{
    skJson json = skJson_Create();
    json->json = j->json[index];
    return json;
}

int skJson_GetArraySize(skJson j)
{
    return j->json.size();
}

bool skJson_HasKey(skJson j, const char* key)
{
    return j->json.contains(key);
}

bool skJson_SaveToFile(skJson j, const char* filename)
{
    if (j == nullptr || filename == nullptr)
    {
        return false;
    }

    try
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            return false;
        }

        file << j->json.dump(4); // dump with 4-space indentation
        file.close();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

skJson skJson_LoadFromFile(const char* filename)
{
    if (filename == nullptr)
    {
        return nullptr;
    }

    try
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            return nullptr;
        }

        // Create a new skJson object
        skJson j = skJson_Create();

        // Parse the file contents
        nlohmann::json parsedskJson = nlohmann::json::parse(file);

        // Set the parsed JSON to our skJson object
        j->json = parsedskJson;

        return j;
    }
    catch (...)
    {
        // In case of any parsing or file reading error
        return nullptr;
    }
}

}

nlohmann::json skJson_GetskJson(skJson j)
{
    return j->json;
}

void skJson_SetskJson(skJson j, const nlohmann::json& json)
{
    j->json = json;
}
