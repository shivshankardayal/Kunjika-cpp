#ifndef USDIGI_H
#define USDIGI_H

#include <memory>

#include "home.h"
#include "login.h"
#include "registration.h"
#include "globals.h"
#include "couchbase_pool.h"

using std::shared_ptr;

namespace kunjika
{
class kunjika : public cppcms::application
{
    shared_ptr<Couchbase_Pool> cbp;

public:
    kunjika(cppcms::service& s, shared_ptr<Couchbase_Pool> cbp);
    void home();
};
}
#endif
