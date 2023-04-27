#include "drpc.h"
#include "test_rpcs.h"
#include <iostream>
#include <map>
#include <mutex>
#include <string>

std::mutex l;

class Test
{
private:
    drpc_server *s;
    std::string id;
    std::map<uint32_t, int> seeds;

public:
    Test()
    {
        drpc_host d{"localhost", 8020};
        s = new drpc_server(d, (void *)this);
        id = "Tester";

        s->publish_endpoint("foo", (void *)this->foo);
        std::cout << "starting server" << std::endl;
        s->run_server();
    }

    static void foo(Test *t, drpc_msg &m)
    {
        l.lock();
        basic_request *breq = (basic_request *)m.req->args;
        if (t->seeds.find(breq->seed) != t->seeds.end())
        {
            // setup reply
            basic_reply *brep = (basic_reply *)m.rep->args;
            brep->status = t->seeds[breq->seed];
            m.rep->len = sizeof(basic_reply);
            l.unlock();
            return;
        }

        std::cout << t->id << " Received a message from " << breq->name << std::endl;

        // setup reply
        int status = 0xf;
        basic_reply *brep = (basic_reply *)m.rep->args;
        brep->status = status;
        m.rep->len = sizeof(basic_reply);

        t->seeds[breq->seed] = status;
        l.unlock();
        return;
    }

    ~Test()
    {
        delete s;
    }
};

int main()
{
    Test t;
    return 0;
}