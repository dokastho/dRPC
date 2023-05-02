#include "drpc.h"
#include "test_rpcs.h"
#include <thread>
#include <iostream>
#include <cassert>

class Killer
{
private:
    drpc_server *s;
    std::thread s_t;

public:
    Killer(drpc_host h)
    {
        s = new drpc_server(h, this);
        std::cout << "starting server" << std::endl;
        std::thread t(&drpc_server::run_server, s);
        // t.detach();
        s_t = std::move(t);
    }

    void kill()
    {
        s->kill();
    }
    
    bool is_killed()
    {
        return !s->is_alive();
    }

    ~Killer()
    {
        s_t.join();
        std::cout << "server is killed" << std::endl;
        delete s;
    }
};

int main()
{
    drpc_host h{"localhost", 8023};

    Killer k(h);

    k.kill();

    assert(k.is_killed());

    return 0;
}