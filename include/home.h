#ifndef HOME_H
#define HOME_H

#include "globals.h"
#include <cppcms/view.h>
#include <string>

using std::string;

namespace content
{
struct home : public cppcms::base_content
{
    string base_path;
    string google_recpatcha_public_key;
    string registration_error;
    string analytics_key;
    string domain;
    struct kunjika::counters sc;
};
}

#endif // end of home.h