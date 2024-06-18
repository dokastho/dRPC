#include "drpc.h"
#include "test_rpcs.h"
#include <cstddef>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <string.h>

std::mutex m;
bool did_pass = true;

void func(unsigned int i)
{
    drpc_host h{"localhost", 8021};
    drpc_client c;

    std::string s = "concurrent test client #" + std::to_string(i);
    basic_request breq{"", 0};
    strcpy(breq.name, s.c_str());
    breq.seed = i;
    basic_reply brep{0, 0};
    rpc_arg_wrapper req{(void *)&breq, sizeof(basic_request)};
    rpc_arg_wrapper rep{(void *)&brep, sizeof(basic_reply)};

    while (brep.status == 0)
    {
        c.Call(h, "foo", &req, &rep);
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
    int count = 500;

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