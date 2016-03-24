#ifndef CONFIG_H_
#define CONFIG_H_

#include <cppcms/json.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include <pthread.h>

// This file is meant to be included only in main.cpp
// including this anywhere else will cause redefinition
// error. This ensures that only one config object exists.

using std::string;
using std::cout;
using std::cerr;
using std::endl;

namespace kunjika
{
static cppcms::json::value config;

void parse_config(const string& config_file_path, cppcms::json::value& config);
}

#endif // end of config.h