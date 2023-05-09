#include "drpc.h"
#include "test_rpcs.h"
#include <thread>
#include <iostream>
#include <cassert>

#define OK 0xf
#define ERR 0x0

class Deaf
{
private:
    drpc_server *s;
    std::thread s_t;

public:
    Deaf(drpc_host h)
    {
        s = new drpc_server(h, this);
        s->publish_endpoint("Kill", (void *)Deaf::Deafen);
        std::thread t(&drpc_server::run_server, s);
        // t.detach();
        s_t = std::move(t);
    }

    static void Deafen(Deaf *d, drpc_msg &m)
    {
        basic_reply *r = (basic_reply *)m.rep->args;
        d->s->kill();
        r->status = OK;
    }

    int call_deafen(drpc_host &h)
    {
        drpc_client c;
        rpc_arg_wrapper req{nullptr, 0};
        rpc_arg_wrapper rep{nullptr, 0};
        basic_reply r{0, ERR};
        rep.args = &r;
        rep.len = sizeof(basic_reply);
        c.Call(h, "Kill", &req, &rep);
        return r.status;
    }

    bool is_killed()
    {
        return !s->is_alive();
    }

    ~Deaf()
    {
        s->kill();
        s_t.join();
        delete s;
    }
};

int main()
{
    drpc_host h1{"localhost", 8023};
    drpc_host h2{"localhost", 8024};

    Deaf d1(h1);
    Deaf d2(h2);

    assert(!d1.is_killed());
    assert(!d2.is_killed());

    int ret;
    ret = d1.call_deafen(h2);
    assert(ret == OK);

    assert(!d1.is_killed());
    assert(d2.is_killed());

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    ret = d1.call_deafen(h2);
    assert(ret == ERR);
    ret = d2.call_deafen(h1);
    assert(ret == OK);

    assert(d1.is_killed());
    assert(d2.is_killed());

    std::cout << "passed" << std::endl;

    return 0;
}