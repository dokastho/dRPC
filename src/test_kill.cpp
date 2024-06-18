#include "drpc.h"
#include <iostream>
#include <cassert>

class Killer
{
private:
    drpc_server *s;

public:
    Killer(drpc_host h)
    {
        s = new drpc_server(h, this);
        std::cout << "starting server" << std::endl;
        s->start();
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
        delete s;
        std::cout << "server is killed" << std::endl;
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