#include <cppcms/http_response.h>
#include <booster/log.h>

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <sstream>
#include <memory>
#include <map>
#include <cstring>
#include <cstdlib>
#include <errno.h>

extern "C" {
#include <curl/curl.h>

#include <openssl/sha.h>
}

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>

#include <libscrypt.h>

#include "registration.h"
#include "globals.h"
#include "base64.h"
#include "couchbase_pool.h"

using std::map;
using std::shared_ptr;

namespace kunjika
{
registration::registration(cppcms::service& s, shared_ptr<Couchbase_Pool> cbp)
    : cppcms::application(s)
    , cbp(cbp)
{
    dispatcher().assign("", &registration::validate_input, this);
    mapper().assign("");

    dispatcher().assign("/verify_email(.*)", &registration::verify_email, this, 1);
    mapper().assign("verify_email", "/verify_email{1}");
};

void registration::validate_input()
{
    if(request().request_method() == "POST") {
        registration_content::registration_data e;
        fname = request().post("fname");
        lname = request().post("lname");
        password = request().post("password");
        confirm = request().post("confirm");
        email = request().post("email");
        g_recaptcha_response = request().post("g-recaptcha-response");

        e.base_path = settings().get<string>("base_path");
        e.google_recpatcha_public_key = settings().get<string>("recaptcha_public_key");

        std::cout << fname << std::endl;
        std::cout << lname << std::endl;
        std::cout << password << std::endl;
        std::cout << confirm << std::endl;
        std::cout << email << std::endl;
        std::cout << g_recaptcha_response << std::endl;

        if(fname.length() < 3 || fname.length() > 32) {
            e.registration_error = "First name must be greater than 2 and less than 33 characters.";
            render("registration_error", e);
        }

        if(lname.length() < 3 || lname.length() > 32) {
            e.registration_error = "Last name must be greater than 2 and less than 33 characters.";
            render("registration_error", e);
        }

        if(password.length() < 8 || password.length() > 32) {
            e.registration_error = "Password must be greater than 7 and less than 33 characters.";
            render("registration_error", e);
        }

        if(confirm.length() < 8 || confirm.length() > 32) {
            e.registration_error = "Password confirmation must be greater than 7 and less than 33 characters and must "
                                   "be same as password.";
            render("registration_error", e);
        }

        if(email.length() < 5 || email.length() > 128) {
            e.registration_error = "Email confirmation must be greater than 4 and less than 129 characters and must "
                                   "be a valid email.";
            render("registration_error", e);
        }

        std::regex txt_regex("([A-Za-z]{3,32})");
        std::smatch base_match;

        if(!std::regex_match(fname, base_match, txt_regex)) {
            e.registration_error = "First name must contain only alphabets.";
            render("registration_error", e);
        }

        if(!std::regex_match(lname, base_match, txt_regex)) {
            e.registration_error = "Last name must contain only alphabets.";
            render("registration_error", e);
        }

        if(password != confirm) {
            e.registration_error = "Password and it's confirmation do not match.";
            render("registration_error", e);
        }

        // first we validate google recaptcha response
        validate_google_recaptcha();
        // first we see if email is already registered or verification is pending
        if(check_email_existence())
            return;
        // put data in pending verification table
        put_reginfo_verification_email();
        render("registration_email_sent", e);

    } else {
        response().status(cppcms::http::response::method_not_allowed);
    }
}

int registration::check_email_existence()
{
    BOOSTER_INFO(app_name) << "Checking email existence";
    registration_content::registration_data e;

    // DO NOT FORGET TO PUSH BACH THE HANDLE TO POOL
    shared_ptr<Couchbase::Client> ch = cbp->pop();
    Couchbase::QueryCommand qcmd("SELECT email "
                                 "FROM `kunjika` where type__=\"up\" and email=\"" +
                                 request().post("email") + "\";");
    Couchbase::Status status;

    Couchbase::Query q(*(ch.get()), qcmd, status);
    if(!(status && q.status())) {
        e.registration_error = "Something went wrong. Please try again. Webmaster has been notified.";
        render("registration_error", e);
    }

    // we will have only one row in the query
    for(auto row : q) {
        e.registration_error = "This email has already been registered.";
        e.base_path = settings().get<string>("base_path");
        e.google_recpatcha_public_key = settings().get<string>("recaptcha_public_key");
        render("registration_error", e);
    }

    cbp->push(ch);
    return 0;
}

void registration::validate_google_recaptcha()
{
    if(!kunjika::validate_google_recaptcha(g_recaptcha_response,
                                  settings().get<string>("recaptcha_private_key"),
                                  request().remote_addr(),
                                  settings().get<string>("g_recaptcha_url"))) {
        registration_content::registration_data e;
        e.base_path = settings().get<string>("base_path");
        e.registration_error = "Recaptcha test failed.";
        e.google_recpatcha_public_key = settings().get<string>("recaptcha_public_key");
        render("registration_error", e);
    }
}

void registration::put_reginfo_verification_email()
{
    char hashed_passwd[1024];
    char err[1024];
    registration_content::registration_data e;
    char string_to_be_checksumed[256];
    string uuid;
    char checksum[256] = { 0 };
    boost::uuids::random_generator generator;

    using boost::lexical_cast;
    using boost::bad_lexical_cast;

    boost::uuids::uuid uuid_tmp = generator();
    e.base_path = settings().get<string>("base_path");
    e.google_recpatcha_public_key = settings().get<string>("recaptcha_public_key");

    try {
        uuid = lexical_cast<string>(uuid_tmp);
    } catch(const bad_lexical_cast& e) {
        // TODO: render an appropriate page
        BOOSTER_ERROR(app_name) << e.what();
        return;
    }

    verification_token = email + uuid;
    strcpy(string_to_be_checksumed, verification_token.c_str());

    BOOSTER_INFO(app_name) << "String for checksum is: " << (char*)string_to_be_checksumed;

    // SHA512((const unsigned char*)string_to_be_checksumed, strlen((const char*)string_to_be_checksumed), checksum);
    libscrypt_hash(checksum, string_to_be_checksumed, SCRYPT_N, SCRYPT_r, SCRYPT_p);
    BOOSTER_INFO(app_name) << string_to_be_checksumed;
    verification_token = base64_encode((const unsigned char*)checksum, strlen(checksum));
    BOOSTER_INFO(app_name) << checksum;
    CURL* curl = curl_easy_init();
    char* url_encoded_verification_token = curl_easy_escape(curl, verification_token.c_str(), 0);
    if(!url_encoded_verification_token) {
        e.registration_error = "Something went wrong. Webmaster has been notified.";
        render("registration_error", e);
    }

    BOOSTER_INFO(app_name) << "Verification token is: " << verification_token;

    curl_easy_cleanup(curl);
    libscrypt_hash(hashed_passwd, password.c_str(), SCRYPT_N, SCRYPT_r, SCRYPT_p);

    BOOSTER_ERROR(app_name) << strerror_r(errno, err, 256);
    BOOSTER_INFO(app_name) << hashed_passwd;

    send_verification_email();

    cppcms::json::value doc;
    doc["email"] = email;
    doc["fname"] = fname;
    doc["lname"] = lname;
    doc["passwd"] = string(hashed_passwd);
    doc["verification_token"] = verification_token;
    doc["type__"] = "pv";
    auto ch = cbp->pop();

    BOOSTER_INFO(app_name) << doc.save();
    string key = string("pv_") + email;
    StoreCommand<LCB_SET> st(key, doc.save());
    st.expiry(settings().get<unsigned long long>("verification_data_ttl"));
    auto sres = ch->upsert(st);

    BOOSTER_INFO(app_name) << std::hex << sres.cas();

    cbp->push(ch);

    verification_token = url_encoded_verification_token;
    free(url_encoded_verification_token);

    if(sres.cas())
        send_verification_email();
    else {
        registration_content::registration_data e;
        e.registration_error = "Something went wrong. Webmaster has been notified.";
        render("registration_error", e);
    }
}

void registration::send_verification_email()
{
    CURLcode res = CURLE_OK;
    struct curl_slist* recipients = NULL;
    struct upload_status upload_ctx;
    int handle_count = 0;

    upload_ctx.lines_read = 0;

    CURL* curl = curl_easy_init();
    CURLM* curlm = curl_multi_init();
    if(curl) {
        // the verification url is not expected to exceed 512 characters
        payload_text[6] = (char*)malloc(sizeof(char) * 512);
        strcpy(payload_text[6], (settings().get<string>("verification_url") + verification_token).c_str());
        strcat(payload_text[6], "\r\n");

        BOOSTER_INFO(app_name) << "Verification url is: " << payload_text[6];

        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, settings().get<string>("sender_email").c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, settings().get<string>("sender_password").c_str());

        /* This is the URL for your mailserver. Note the use of port 587 here,
         * instead of the normal SMTP port (25). Port 587 is commonly used for
         * secure mail submission (see RFC4403), but you should use whatever
         * matches your server configuration. */
        curl_easy_setopt(curl, CURLOPT_URL, settings().get<string>("smtp_address").c_str());

        /* In this example, we'll start with a plain text connection, and upgrade
         * to Transport Layer Security (TLS) using the STARTTLS command. Be careful
         * of using CURLUSESSL_TRY here, because if TLS upgrade fails, the transfer
         * will continue anyway - see the security discussion in the libcurl
         * tutorial for more details. */
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

        /* If your server doesn't have a valid certificate, then you can disable
         * part of the Transport Layer Security protection by setting the
         * CURLOPT_SSL_VERIFYPEER and CURLOPT_SSL_VERIFYHOST options to 0 (false).*/
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        /* That is, in general, a bad idea. It is still better than sending your
        * authentication details in plain text though.  Instead, you should get
        * the issuer certificate (or the host certificate if the certificate is
        * self-signed) and add it to the set of certificates that are known to
        * libcurl using CURLOPT_CAINFO and/or CURLOPT_CAPATH. See docs/SSLCERTS
        * for more information. */
        // curl_easy_setopt(curl, CURLOPT_CAINFO, "./cert.pem");

        /* Note that this option isn't strictly required, omitting it will result
         * in libcurl sending the MAIL FROM command with empty sender data. All
         * autoresponses should have an empty reverse-path, and should be directed
         * to the address in the reverse-path which triggered them. Otherwise,
         * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
         * details.
         */
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);

        /* Add two recipients, in this particular case they correspond to the
         * To: and Cc: addressees in the header, but they could be any kind of
         * recipient. */
        recipients = curl_slist_append(recipients, TO);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        /* We're using a callback function to specify the payload (the headers and
         * body of the message). You could just use the CURLOPT_READDATA option to
         * specify a FILE pointer to read from. */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* Since the traffic will be encrypted, it is very useful to turn on debug
         * information within libcurl to see what is happening during the transfer.
         */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_multi_add_handle(curlm, curl);
        CURLMcode code;
        //        while(1) {
        //            code = curl_multi_perform(curlm, &handle_count);
        //
        //            if(handle_count == 0) {
        //                break;
        //            }
        //        }
        //
        //        /* Send the message */
        //        // res = curl_easy_perform(curl);
        //
        //        /* Check for errors */
        //        if(res != CURLE_OK)
        //            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        /* Free the list of recipients */
        curl_slist_free_all(recipients);

        /* Always cleanup */
        curl_easy_cleanup(curl);
        curl_multi_cleanup(curlm);
        free(payload_text[6]);
    }
}

void registration::verify_email(string url)
{
    registration_content::registration_data e;
    if(request().request_method() == "GET") {
        bool flag = false;
        e.base_path = settings().get<string>("base_path");
        e.google_recpatcha_public_key = settings().get<string>("recaptcha_public_key");
        string email;
        string fname;
        string lanme;
        string password;

        e.base_path = settings().get<string>("base_path");
        e.google_recpatcha_public_key = settings().get<string>("recaptcha_public_key");
        auto ch = cbp->pop();
        Couchbase::QueryCommand qcmd("SELECT email, fname, lname, passwd "
                                     "FROM `kunjika` where type__=\"pv\" and verification_token=\"" +
                                     request().get("verification_token") + "\";");
        registration_content::registration_data e;
        Couchbase::Status status;

        Couchbase::Query q(*(ch.get()), qcmd, status);
        if(!(status && q.status())) {
            e.registration_error = "Your verification code might have expired.";
            render("registration_error", e);
        }

        // we will have only one row in the query
        cppcms::json::value user_doc;
        for(auto row : q) {
            std::istringstream iss(row.json());
            user_doc.load(iss, false);
        }

        try {
            user_doc.get<string>("email"); // just to check if we actually got something
            user_doc["type__"] = "up";
            Couchbase::CounterCommand ccmd(1);
            ccmd.key("ucount");

            auto sres = ch->counter(ccmd);

            size_t uid = sres.value();

            if(uid) {
                user_doc["uid"] = uid;
                auto ures = ch->upsert(std::to_string(uid), user_doc.save());
                BOOSTER_INFO(app_name) << std::hex << sres.cas();
                if(!sres.cas()) {
                    e.registration_error = "Something went wrong. Please try again. Webmaster has been"
                                           "notified.";
                    render("registration_error", e);
                }
            }

            ch->remove("pv_" + user_doc.get<string>("email"));
            e.base_path = settings().get<string>("base_path");
            render("email_verified", e);
            cbp->push(ch);
        } catch(...) {
            cbp->push(ch);
            e.registration_error = "Something went wrong. Please try again. Webmaster has been notified.";
            render("registration_error", e);
        }
    } else {
        response().status(cppcms::http::response::method_not_allowed);
    }
}
}
