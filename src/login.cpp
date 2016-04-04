#include <booster/log.h>

#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <errno.h>

#include <cassandra.h>

extern "C" {
#include <curl/curl.h>

#include <openssl/sha.h>
}

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>

#include <libscrypt.h>

#include "login.h"
#include "globals.h"
#include "base64.h"
#include "couchbase_pool.h"

using std::map;
using std::shared_ptr;

namespace kunjika
{
login::login(cppcms::service& s, shared_ptr<Couchbase_Pool> cbp)
    : cppcms::application(s)
    , cbp(cbp)
{
    dispatcher().assign("", &login::handle_login, this);
    mapper().assign("");
}

void login::handle_login()
{
    if(request().request_method() == "GET") {
        login_content::login_data c;

        c.base_path = settings().get<std::string>("base_path");
        c.google_recpatcha_public_key = settings().get<std::string>("recaptcha_public_key");
        render("login_view", c);
    }
// else if(request().request_method() == "POST") {
//        login_content::login_data l;
//        string email = request().post("email");
//        string password = request().post("password");
//        bool flag = false;
//
//        shared_ptr<Couchbase::Client> ch = cbp->pop();
//        Couchbase::QueryCommand qcmd("SELECT uid, email, fname, lname, passwd "
//                                     "FROM `kunjika` where type__=\"up\" and email=\"" +
//                                     email + "\";");
//        Couchbase::Status status;
//
//        Couchbase::Query q(*(ch.get()), qcmd, status);
//        if(!(status && q.status())) {
//            l.registration_error = "Something went wrong. Please try again. Webmaster has been notified.";
//            render("registration_error", l);
//        }
//
//        // we will have only one row in the query
//        try {
//            cppcms::json::value user_doc;
//
//            for(auto row : q) {
//                std::istringstream iss(row.json());
//                BOOSTER_INFO(app_name) << row.json();
//                user_doc.load(iss, false);
//            }
//
//            BOOSTER_INFO(app_name) << user_doc.save();
//            cbp->push(ch);
//
//            // double copying because c_str on string wont work as libcrypt modifies it
//            char password1[256] = { 0 };
//            size_t uid = user_doc.get<size_t>("uid");
//            string password2 = user_doc.get<string>("passwd");
//            string fname = user_doc.get<string>("fname");
//            string lname = user_doc.get<string>("lname");
//            string email = user_doc.get<string>("email");
//
//            strcpy(password1, password2.c_str());
//
//            BOOSTER_INFO(app_name) << "scrypt check result is " << libscrypt_check(password1, password.c_str());
//
//            BOOSTER_INFO(app_name) << "UID: " << uid << "First Name: " << fname
//                                   << "Last Name: " << lname;
//            session().set("uid", uid);
//            session().set("fname", fname);
//            session().set("lname", lname);
//            session().set("email", email);
//
//            l.base_path = settings().get<std::string>("base_path");
//            l.google_recpatcha_public_key = settings().get<std::string>("recaptcha_public_key");
//            free(ui);
//
//            if(libscrypt_check((char*)password2.c_str(), password.c_str()) > 0) {
//                response().set_redirect_header(l.base_path + "/profile");
//            } else {
//                l.login_error = "Either email or password is wrong.";
//                render("email_not_found", l);
//            }
//        } catch(const std::exception& e) {
//            BOOSTER_INFO(app_name) << e.what();
//        }
//    } else {
//        response().status(cppcms::http::response::method_not_allowed);
//    }
}
}