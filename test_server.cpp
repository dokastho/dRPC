#include "drpc.h"
#include <iostream>
#include <string>
#include "test_rpcs.h"

class Test
{
    private:
    drpc_server *s;
    std::string id;


    public:
    Test()
    {
        drpc_host d{"localhost", 8020};
        s = new drpc_server(d, (void*)this);
        id = "Tester";

        s->publish_endpoint("foo", (void*)this->foo);
        std::cout << "starting server" << std::endl;
        s->run_server();
    }

    static void foo(Test *t, drpc_msg &m)
    {
        basic_request* breq = (basic_request*)m.req->args;
        std::cout << t->id << " Received a message from " << breq->name << std::endl;

        // setup reply
        basic_reply* brep = (basic_reply*)m.rep->args;
        brep->status = 0xf;
        m.rep->len = sizeof(basic_reply);
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