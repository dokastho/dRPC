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
    int id;

public:
    Test()
    {
        drpc_host d{"localhost", 8021};
        s = new drpc_server(d, (void *)this);
        id = 1;

        s->publish_endpoint("foo", (void *)this->foo);
        std::cout << "starting server" << std::endl;
        s->start();
    }

    static void foo(Test *t, drpc_msg &m)
    {
        basic_request *breq = (basic_request *)m.req->args;
        l.lock();
        std::cout << t->id << " Received a message from " << breq->name << std::endl;
        l.unlock();

        // setup reply
        int status = 0xf;
        basic_reply *brep = (basic_reply *)m.rep->args;
        brep->host = t->id;
        brep->status = status;
        m.rep->len = sizeof(basic_reply);
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TIMEOUT));
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
    std::string cmd = "";
    do
    {
        std::cout << "enter 'q' to quit\n$ ";
        std::cin >> cmd;
    } while (cmd[0] != 'q');
    return 0;
}