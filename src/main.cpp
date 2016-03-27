#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>

#include <booster/log.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include "kunjika.h"
#include "config.h"
#include "curl/curl.h"
#include "globals.h"
#include "couchbase_pool.h"

using std::string;
using std::shared_ptr;

namespace kunjika
{
void get_counters(shared_ptr<Couchbase_Pool> cbp, struct counters& sc)
{
    auto ch = cbp->pop();

    auto gres = ch->get("ucount");
    sc.ucount = stoull(gres.value().to_string());
    gres = ch->get("acount");
    sc.acount = stoull(gres.value().to_string());
    gres = ch->get("qcount");
    sc.qcount = stoull(gres.value().to_string());
    gres = ch->get("tcount");
    sc.tcount = stoull(gres.value().to_string());

    cbp->push(ch);
}
}

int main(int argc, char** argv)
{
    kunjika::parse_config(argv[2], kunjika::config); // this is the typical configuration file required by cppcms
    curl_global_init(CURL_GLOBAL_ALL);

    shared_ptr<kunjika::Couchbase_Pool> cb_pool = std::make_shared<kunjika::Couchbase_Pool>(50);
    auto ch = cb_pool->pop();
    string query = string("CREATE PRIMARY INDEX ON `") + kunjika::config.get<string>("bucket_name") + string("`");
    auto m = Query::execute(*(ch.get()), query);
    cout << "Index creation: " << endl << "  Status: " << m.status() << endl << "  Body: " << m.body() << endl;

    // initialize counters if they do not exist
    auto sres = ch->insert("ucount", "0");
    BOOSTER_INFO(kunjika::app_name) << "Got status for store. Cas=" << std::hex << sres.cas();

    sres = ch->insert("qcount", "0");
    BOOSTER_INFO(kunjika::app_name) << "Got status for store. Cas=" << std::hex << sres.cas();

    sres = ch->insert("acount", "0");
    BOOSTER_INFO(kunjika::app_name) << "Got status for store. Cas=" << std::hex << sres.cas();

    sres = ch->insert("tcount", "0");
    BOOSTER_INFO(kunjika::app_name) << "Got status for store. Cas=" << std::hex << sres.cas();
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
