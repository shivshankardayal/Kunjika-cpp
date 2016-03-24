#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <iostream>

#include "kunjika.h"

namespace kunjika
{
kunjika::kunjika(cppcms::service& s, shared_ptr<Couchbase_Pool> cbp)
    : cppcms::application(s)
    , cbp(cbp)
{

    dispatcher().assign("", &usdigi::home, this);
    mapper().assign("");

//    attach(new registration(s, cbp), "registration", "/register{1}", "/register((/?.*)?)", 1);
//    attach(new login(s, cbp), "login", "/login{1}", "/login((/?.*)?)", 1);

    mapper().root("/");
}

void usdigi::home()
{
    if(request().request_method() == "GET") {
        content::home c;

        c.base_path = settings().get<std::string>("base_path");
        c.google_recpatcha_public_key = settings().get<std::string>("recaptcha_public_key");
//        render("home", c);
    } else {
        response().status(cppcms::http::response::method_not_allowed);
    }
}
}