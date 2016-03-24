#ifndef COUCHBASE_POOL_H_
#define COUCHBASE_POOL_H_

#include <libcouchbase/couchbase++.h>
#include <libcouchbase/couchbase++/views.h>
#include <libcouchbase/couchbase++/query.h>
#include <libcouchbase/couchbase++/endure.h>
#include <libcouchbase/couchbase++/logging.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <queue>
#include <memory>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::vector;
using std::unique_ptr;

using namespace Couchbase;

#include "config.h"
// Do not include this header other than in main.cpp

namespace usdigi
{
class Couchbase_Pool
{
public:
    /**
     * Create a new pool to use across threads
     * @param options The options used to initialize the instance
     * @param items How many items should be in the pool
     */
    Couchbase_Pool(size_t items = 10)
        : initial_size(0)
    {
        string db_connstr = config.get<string>("db_connstr");

        try {
            items = config.get<unsigned int>("num_db_conn");
        } catch(const std::exception& e) {
            cerr << e.what() << endl;
        }

        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
        
        for(size_t ii = 0; ii < items; ii++) {
            std::shared_ptr<Couchbase::Client> cur(
                new Couchbase::Client(db_connstr));
            Status rv = cur->connect();

            if(!rv.success()) {
                cout << "Couldn't connect to '" << db_connstr << "'. "
                     << "Reason: " << rv << endl;
                exit(EXIT_FAILURE);
            }

            instances.push(std::move(cur));
            initial_size++;
        }
    }
    virtual ~Couchbase_Pool(){};

    /**Get an instance from the pool. You should call #push() when you are
     * done with the instance
     * @return an lcb_t instance */
    std::shared_ptr<Couchbase::Client> pop()
    {

        // Need to lock the mutex to the pool structure itself
        pthread_mutex_lock(&mutex);

        while(instances.empty()) {
            pthread_cond_wait(&cond, &mutex);
        }

        std::shared_ptr<Couchbase::Client> ret(std::move(instances.front()));
        instances.pop();
        pthread_mutex_unlock(&mutex);

        // Note that the instance itself does not need a mutex as long as it is not
        // used between multiple threads concurrently.
        return ret;
    }
    /**Release an instance back into the pool
     * @param instance The instance to release */
    void push(std::shared_ptr<Couchbase::Client> instance)
    {
        pthread_mutex_lock(&mutex);
        instances.push(std::move(instance));
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

protected:
private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    std::queue<std::shared_ptr<Couchbase::Client> > instances;
    size_t initial_size;
};

} // namespace

#endif // end of couchbase_pool.h