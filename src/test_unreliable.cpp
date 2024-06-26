#include "drpc.h"
#include "test_rpcs.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <string.h>

std::mutex m;
bool did_pass = true;

void func(int i)
{
    drpc_host h{"localhost", 8021};
    drpc_client c;

    std::string s = "concurrent test client #" + std::to_string(i);
    basic_request breq{"", 0};
    strcpy(breq.name, s.c_str());
    breq.seed = (uint32_t)i;
    basic_reply brep{0, 0};
    rpc_arg_wrapper req{(void *)&breq, sizeof(basic_request)};
    rpc_arg_wrapper rep{(void *)&brep, sizeof(basic_reply)};

    for (size_t i = 0; i < 10; i++)
    {
        c.Call(h, "foo", &req, &rep);
        if (brep.status == 0xf)
        {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
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
    int count = 0x200;

    for (int i = 0; i < count; i++)
    {
        std::thread t(&func, i);
        threads.push_back(std::move(t));
    }

    for (int i = 0; i < count; i++)
    {
        threads[(size_t)i].join();
    }

    assert(did_pass);

    return 0;
}