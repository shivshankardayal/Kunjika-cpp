#ifndef GLOBALS_H
#define GLOBALS_H

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

using std::string;
using std::shared_ptr;

namespace kunjika
{
struct MemoryStruct
{
    char* memory;
    size_t size;
};

static const char* app_name = "kunjika";

size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp);
int validate_google_recaptcha(const string& g_recaptcha_response,
                                     const string& recaptcha_secret_key,
                                     const string& remote_addr,
                                     const string& recaptcha_url);


struct counters {
    size_t qcount;
    size_t acount;
    size_t tcount;
    size_t ucount;
};

void get_counters(shared_ptr<Couchbase_Pool> cbp, struct counters& sc);
}

#endif // end of globals.h