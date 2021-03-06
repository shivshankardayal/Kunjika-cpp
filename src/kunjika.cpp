#include "kunjika.h"

namespace kunjika
{
kunjika::kunjika(cppcms::service& s, shared_ptr<Couchbase_Pool> cbp)
    : cppcms::application(s)
    , cbp(cbp)
{

    dispatcher().assign("", &kunjika::home, this);
    mapper().assign("");
    
    attach(new registration(s, cbp), "registration", "/register{1}", "/register((/?.*)?)", 1);
    attach(new login(s, cbp), "login", "/login{1}", "/login((/?.*)?)", 1);

    mapper().root("/");
}

void kunjika::home()
{
    if(request().request_method() == "GET") {
        content::home c;
        try {
            c.greeting = session().get<bool>("greeting");
        } catch(const std::exception& e) {
            c.greeting = false;
            session().set("greeting", true);
        }

        BOOSTER_INFO(app_name) << "greeting is " << c.greeting;
        get_counters(cbp, c.sc);
        c.base_path = settings().get<std::string>("base_path");
        c.google_recpatcha_public_key = settings().get<std::string>("recaptcha_public_key");
        render("index", c);
    } else {
        response().status(cppcms::http::response::method_not_allowed);
    }
}
}