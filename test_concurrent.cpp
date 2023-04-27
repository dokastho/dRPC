#include "drpc.h"
#include "test_rpcs.h"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

std::mutex m;
bool did_pass = true;

void func(int i)
{
    drpc_host h{"localhost", 8020};
    drpc_client c;

    std::string s = "basic test client #" + std::to_string(i);
    basic_request breq;
    strcpy(breq.name, s.c_str());
    basic_reply brep{0};
    rpc_arg_wrapper req{(void *)&breq, sizeof(basic_request)};
    rpc_arg_wrapper rep{(void *)&brep, sizeof(basic_reply)};

    c.Call(h, "foo", &req, &rep);
    if (brep.status != 0xf)
    {
        m.lock();
        did_pass = false;
        std::cout << "fail #" << i << std::endl;
        m.unlock();
    }
    
}

int main()
{
    std::vector<std::thread> threads;
    size_t count = 100;

    for (size_t i = 0; i < count; i++)
    {
        std::thread t(&func, i);
        threads.push_back(std::move(t));
    }

    for (size_t i = 0; i < count; i++)
    {
        threads[i].join();
    }

    assert(did_pass);

    return 0;
}