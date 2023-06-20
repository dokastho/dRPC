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
        std::cout << "starting server" << std::endl;
        s->start();
    }

    static void foo(Server *t, drpc_msg &m)
    {
        basic_request *breq = (basic_request *)m.req->args;
        basic_reply *brep = (basic_reply *)m.rep->args;
        std::cout << t->id << " Received a message from " << breq->name << std::endl;
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


int main()
{
    Server s;
    drpc_client c;

    basic_request breq{"basic test client", 0};
    basic_reply brep{0, 0};
    rpc_arg_wrapper req{(void*)&breq, sizeof(basic_request)};
    rpc_arg_wrapper rep{(void*)&brep, sizeof(basic_reply)};

    c.Call(shost, "foo", &req, &rep);

    // timeout for 2s 
    std::this_thread::sleep_for(std::chrono::milliseconds(4 * DEFAULT_TIMEOUT));
    assert(brep.status == 0);
    std::cout << "passed" << std::endl;
    return 0;
}