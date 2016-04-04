#include <curl/curl.h>
#include <cppcms/json.h>

#include <cstdlib>
#include <iostream>
#include <sstream>

#include "globals.h"

namespace kunjika
{
size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        // out of memory!
        std::cerr << "not enough memory (realloc returned NULL)" << std::endl;
        exit(EXIT_FAILURE);
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    // cout << mem->memory<< endl;
    return realsize;
}

int validate_google_recaptcha(const string& g_recaptcha_response,
                                     const string& recaptcha_secret_key,
                                     const string& remote_addr,
                                     const string& recaptcha_url)
{
    CURL* curl_handle;
    CURLcode res;

    struct kunjika::MemoryStruct chunk;

    chunk.memory = (char*)malloc(1); // will be grown as needed by the realloc above
    chunk.size = 0;                  // no data at this point

    curl_handle = curl_easy_init();

    string post_fields{};

    post_fields = "response=" + g_recaptcha_response + "&secret=" + recaptcha_secret_key +
                  "&remoteip=" + remote_addr;

    curl_easy_setopt(curl_handle, CURLOPT_URL, recaptcha_url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, kunjika::WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post_fields.c_str());

    res = curl_easy_perform(curl_handle);
    // cleanup curl stuff
    curl_easy_cleanup(curl_handle);

    if(res != CURLE_OK) {
        return res;
    }

    std::istringstream recaptcha_response(chunk.memory);
    //BOOSTER_INFO(app_name) << chunk.memory;

    free(chunk.memory);

    std::unique_ptr<cppcms::json::value> recaptcha_response_json(new cppcms::json::value());

    recaptcha_response_json->load(recaptcha_response, true);

    return recaptcha_response_json->get<bool>("success");
}
}