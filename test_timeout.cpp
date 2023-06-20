#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cassert>

#include "drpc.h"
#include "test_rpcs.h"
#include "test_helpers.h"

#define nreqs 500
std::vector<bool> passes;

void func(int me)
{
    drpc_client c;
    basic_request breq{"basic test client", 0};
    basic_reply brep{0, 0};
    rpc_arg_wrapper req{(void*)&breq, sizeof(basic_request)};
    rpc_arg_wrapper rep{(void*)&brep, sizeof(basic_reply)};

    int status = 1;
    while (status != 0)
    {
        status = c.Call(shost, "foo", &req, &rep);
    }
    passes[me] = true;
}

int main()
{
    Server s;
    drpc_client c;

    basic_request breq{"basic test client", 0};
    basic_reply brep{0, 0};
    rpc_arg_wrapper req{(void*)&breq, sizeof(basic_request)};
    rpc_arg_wrapper rep{(void*)&brep, sizeof(basic_reply)};

    std::cout << "Test: one request that times out" << std::endl;
    c.Call(shost, "slow", &req, &rep);

    // timeout for 2s 
    std::this_thread::sleep_for(std::chrono::milliseconds(4 * DEFAULT_TIMEOUT));
    assert(brep.status == 0);
    std::cout << "passed" << std::endl;
    std::cout << "Test: many requests that time out" << std::endl;

    std::vector<std::thread> threads;

    for (size_t i = 0; i < nreqs; i++)
    {
        std::thread t(&func, i);
        threads.push_back(std::move(t));
        passes.push_back(false);
    }
    for (size_t i = 0; i < nreqs; i++)
    {
        threads[i].join();
    }
    
    bool did_pass = true;

    for (size_t i = 0; i < nreqs; i++)
    {
        did_pass = did_pass && passes[i];
    }
    assert(did_pass);

    std::cout << "passed" << std::endl;
    return 0;
}