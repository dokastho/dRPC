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
        std::cout << "Hello World! from " << t->id << std::endl;

        // setup reply
        basic_reply* b = (basic_reply*)m.rep->args;
        b->status = 0xf;
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