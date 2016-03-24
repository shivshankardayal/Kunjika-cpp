#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <cstdlib>

namespace kunjika
{
struct MemoryStruct
{
    char* memory;
    size_t size;
};

static const char* app_name = "kunjika";

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        // out of memory!
        BOOSTER_ERROR(app_name) << "not enough memory (realloc returned NULL)";
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    // cout << mem->memory<< endl;
    return realsize;
}
}