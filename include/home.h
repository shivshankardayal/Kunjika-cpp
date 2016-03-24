#ifndef HOME_H
#define HOME_H

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
    size_t qcount;
    size_t acount;
    size_t ucount;
    size_t tcount;
    string analytics_key;
    string domain;
};
}

#endif // end of home.h