#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>

#include <booster/log.h>

#include <iostream>
#include <fstream>
#include <memory>

#include "kunjika.h"
#include "config.h"
#include "couchbase_pool.h"

using std::string;
using std::shared_ptr;

int main(int argc, char** argv)
{
    usdigi::parse_config(argv[2], usdigi::config); // this is the typical configuration file required by cppcms
    curl_global_init(CURL_GLOBAL_ALL);

    shared_ptr<usdigi::Couchbase_Pool> cb_pool = std::make_shared<usdigi::Couchbase_Pool>(50);
    auto ch = cb_pool->pop();
    auto m = Query::execute(*(ch.get()), "CREATE PRIMARY INDEX ON `" + config.get<string>("bucket_name") + "`");
    cout << "Index creation: " << endl
         << "  Status: " << m.status() << endl
         << "  Body: " << m.body() << endl;
         
    auto sres = ch->insert("ucount", "0");
    BOOSTER_INFO(usdigi::app_name) << "Got status for store. Cas=" << std::hex << sres.cas();
    
    auto sres = ch->insert("qcount", "0");
    BOOSTER_INFO(usdigi::app_name) << "Got status for store. Cas=" << std::hex << sres.cas();
    
    auto sres = ch->insert("acount", "0");
    BOOSTER_INFO(usdigi::app_name) << "Got status for store. Cas=" << std::hex << sres.cas();
    
    auto sres = ch->insert("tcount", "0");
    BOOSTER_INFO(usdigi::app_name) << "Got status for store. Cas=" << std::hex << sres.cas();
    cb_pool->push(ch);


    try {
        cppcms::service srv(argc, argv);
        srv.applications_pool().mount(cppcms::applications_factory<kunjika::kunjika>(cb_pool));
        srv.run();
    } catch(std::exception const& e) {
        std::cerr << "Failed: " << e.what() << std::endl;
        std::cerr << booster::trace(e) << std::endl;

        // delete cass_instance;
        // we're done with libcurl, so clean it up
        curl_global_cleanup();
        return 1;
    }

    // delete cass_instance;
    curl_global_cleanup();

    return 0;
}
