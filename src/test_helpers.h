#ifndef TEST_HELPERS
#define TEST_HELPERS

#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>

#include "drpc.h"
#include "test_rpcs.h"

drpc_host shost{"localhost", 8021};


class Server
{
private:
    drpc_server *s;
    int id;

public:
    Server()
    {
        s = new drpc_server(shost, (void *)this);
        id = 1;

        s->publish_endpoint("foo", (void *)this->foo);
        s->publish_endpoint("slow", (void *)this->slow);
        std::cout << "starting server" << std::endl;
        s->start();
    }

    static void foo(Server *t, drpc_msg &m)
    {
        basic_request *breq = (basic_request *)m.req->args;
        basic_reply *brep = (basic_reply *)m.rep->args;
        std::cout << t->id << " Received a message from " << breq->name << std::endl;
        brep->status = 0xf;
        return;
    }

    static void slow(Server *t, drpc_msg &m)
    {
        basic_request *breq = (basic_request *)m.req->args;
        basic_reply *brep = (basic_reply *)m.rep->args;
        std::cout << t->id << " Slow: Received a message from " << breq->name << std::endl;
        // timeout for 1s
        std::this_thread::sleep_for(std::chrono::milliseconds(2 * DEFAULT_TIMEOUT));
        brep->status = 0xf;
        return;
    }

    ~Server()
    {
        delete s;
    }
};

#endif