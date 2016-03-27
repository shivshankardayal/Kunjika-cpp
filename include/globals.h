#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <booster/log.h>

#include <memory>
#include <iostream>

#include <cstdlib>

#include "couchbase_pool.h"

using std::shared_ptr;

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

struct counters {
    size_t qcount;
    size_t acount;
    size_t tcount;
    size_t ucount;
};

void get_counters(shared_ptr<Couchbase_Pool> cbp, struct counters& sc);
}

#endif // end of globals.h