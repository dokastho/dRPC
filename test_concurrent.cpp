#include "drpc.h"
#include "test_rpcs.h"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex m;
bool did_pass = true;

void func(int i)
{
    drpc_host h{"localhost", 8020};
    drpc_client c;

    std::string s = "concurrent test client #" + std::to_string(i);
    basic_request breq;
    strcpy(breq.name, s.c_str());
    breq.seed = i;
    basic_reply brep{0};
    rpc_arg_wrapper req{(void *)&breq, sizeof(basic_request)};
    rpc_arg_wrapper rep{(void *)&brep, sizeof(basic_reply)};

    for (size_t i = 0; i < 10; i++)
    {
        c.Call(h, "foo", &req, &rep);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
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
    int count = 100;

    for (int i = 0; i < count; i++)
    {
        std::thread t(&func, i);
        threads.push_back(std::move(t));
    }

    for (int i = 0; i < count; i++)
    {
        threads[i].join();
    }

    assert(did_pass);

    return 0;
}