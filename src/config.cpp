#include <cppcms/json.h>
#include <string>
#include <fstream>
#include <sstream>

namespace kunjika
{
void parse_config(const std::string& config_file_path, cppcms::json::value& config)
{
    std::ifstream file(config_file_path);

    if(file) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        config.load(buffer, true);
        file.close();
    }
}
}