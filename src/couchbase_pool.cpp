#include "couchbase_pool.h"
#include "config.h"

using namespace Couchbase;

namespace kunjika
{

Couchbase_Pool::Couchbase_Pool(size_t nitems)
    : initial_size(0)
{
    string db_connstr = config.get<string>("db_connstr");
    unsigned int num_db_conn = 0;

    try {
        num_db_conn = config.get<unsigned int>("num_db_conn");
        nitems = num_db_conn;
    } catch(const std::exception& e) {
        cerr << e.what() << endl;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    
    for(size_t ii = 0; ii < nitems; ii++) {
        std::shared_ptr<Couchbase::Client> cur(new Couchbase::Client("db_connstr"));
        Status rv = cur->connect();

        if(!rv.success()) {
            cout << "Couldn't connect to '" << db_connstr << "'. "
                 << "Reason: " << rv << endl;
            exit(EXIT_FAILURE);
        }

        instances.push(std::move(cur));
        initial_size++;
    }
    
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

std::shared_ptr<Couchbase::Client> Couchbase_Pool::pop()
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

void Couchbase_Pool::push(std::shared_ptr<Couchbase::Client>& instance)
{
    pthread_mutex_lock(&mutex);
    instances.push(std::move(instance));
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}
}