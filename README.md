ggqa
====
ggqa is a better version of [Memoir](https://github.com/shivshankardayal/memoir).
This uses C++ and new database changes warranted by new release of Couchbase.
Fundamentally, it is QA system like many others, for example, Stackoverflow,
Question2Answer, Answerhub etc. You can see a well-compiled list of QA apps
[here](http://meta.stackexchange.com/questions/2267/stack-exchange-clones).

The development of this system is to satisfy my own itch to write very
high performance web application.

It is envisioned to provide a REST API with this version of QA which was not
their earlier. Once the development is reasonably complete a sandbox version
will be available for testing at https://10hash.com/ggqa

Other than language change from Python to C++ and framework change from
Flask to Seastar the template library had to change as well. I use
Ctemplate(formerly known as Google template) because it is synergetic. DB has
changed from Couchbase to Cassandra and will later change to Scylla as Scylla
matures. I used to stores BLOBs in Couchbase itself but now I will use
Hadoop as a ditributed filesystem(DFS) to store BLOBs which will put less
pressure on DB and is in general better.

### Memoir's Readme

Memoir was port of Kunjika. The migration is because the database design is
being changed completely. The reason of a new name is that if someone wants to work
off Kunjika then this will not have any impact on their work. The new change will
result in a lot less memory consumption from Couchbasse at least to start with
as all of data is being put in one bucket.

Note that in code the app name is still kunjika just the module kunjika.py is
renamed memoir.py. I have decided to keep the same name in code so that people
willing to adapt code from Memoir to Kunjika should have little problem.

#### Note

Right now there is no demo of Kunjika or Memoir being hosted.

### Kunjika's Readme

"It is software made up of bugs."

**Update**
I have stopped developing it temporarily. I will update documentation soon
so that if people want to develop then they should be able to do it easily.
I may migrate code to wheezy.web or Falcon Python frameworks as they are
much faster than Flask but that is to be seen. New features are also to be
implemented. I will resume work as time permits.

It started as Stackoverflow clone but now I have implemented some features which
are not there in Stackoverflow and at the same time many features are not there
as well so calling Kunjika a clone is not appropriate.
It has skills and endorsement features like linked in. I am also going to implement
articles feature which can act as a knowledge base portion for a QA site.
I am using Flask framework so obviously Python as well.
Couchbase for database and Memcahced functionality.

This project has been started to scratch my own itch. OSQA's development has been stopped.
I used LampCMS for sometime but found it buggy. Question2Answer is good and
decent but is in PHP which I do not know so I decided to roll my own.
Askbot could substitute OSQA but all these are turning commercial which is bad
and ugly. Cannot trust how long they will develop open source version. DZone
actually used open source community to test their product and then take away
all of it.

I never learned SQL and the schema thing is a pain for me. So I chose Couchbase
which provided nice replication, auto-sharding and memcached functionality apart
from document based database. So one query for question, one for user and one
for tag and it is done.

The documentation is out of date and I will update that soon.

Kunjika is quite stable now and a beta release has been done.
