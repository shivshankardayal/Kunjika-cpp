#ifndef REGISTER_H
#define REGISTER_H

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/view.h>

#include <memory>
#include <string>

#include "login.h"
#include "couchbase_pool.h"

using std::string;
using std::shared_ptr;
using std::cout;
using std::endl;

namespace registration_content
{
struct registration_data : public login_content::login_data
{
    string registration_error;
    string base_path;
    string google_recpatcha_public_key;
};
}

namespace kunjika
{
class registration : public cppcms::application
{
    string fname;
    string lname;
    string password;
    string confirm;
    string email;
    string verification_token;
    string g_recaptcha_response;
    shared_ptr<Couchbase_Pool> cbp;

public:
    registration(cppcms::service& s, shared_ptr<Couchbase_Pool> cbp);
    void validate_input();
    void validate_google_recaptcha();
    int check_email_existence();
    void put_reginfo_verification_email();
    void send_verification_email();
    void verify_email(string url);
};

#define FROM "<dayal3@yahoo.com>"
#define TO "<shivshankar.dayal@gmail.com>"

static char* payload_text[] = { //"Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n",
    "To: " TO "\r\n",
    "From: " FROM "(Example User)\r\n",
    //                                      "Cc: " CC "(Another example User)\r\n",
    //                                      "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
    //                                      "rfcpedant.example.org>\r\n",
    "Subject: Regstration at Kunjika\r\n",
    "\r\n", /* empty line to divide headers from body, see RFC5322 */
    "Thanks for registering at USDigi. Please complete your email verification by clicking at following link. In case "
    "you cannot click please copy and paste it in your browser. This token will expire after 24 hours.\r\n",
    "\r\n",
    "\r\n",
    "\r\n"
    "Thanks,\r\n",
    "Kunjika Team",
    NULL
};

struct upload_status
{
    int lines_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;

  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }

  data = payload_text[upload_ctx->lines_read];

  if(data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read++;

    return len;
  }

  return 0;
}

}
#endif