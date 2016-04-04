#ifndef LOGIN_H
#define LOGIN_H

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/view.h>
#include <cppcms/session_interface.h>

#include <memory>
#include <string>

#include "couchbase_pool.h"
#include "home.h"

using std::string;
using std::shared_ptr;
using std::cout;
using std::endl;

namespace login_content
{
struct login_data : public content::home
{
    string login_error;
};
}

namespace kunjika
{
class login : public cppcms::application
{
    string fname;
    string lname;
    string password;
    string email;
    string g_recaptcha_response;
    shared_ptr<Couchbase_Pool> cbp;

public:
    login(cppcms::service& s, shared_ptr<Couchbase_Pool> cbp);
    void handle_login();
};
}
#endif // end of login.h