#include "drpc.h"
#include "test_rpcs.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <string.h>

using std::chrono::milliseconds;

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

    while (brep.status != 0xf)
    {
        c.Call(h, "foo", &req, &rep);
    }
}

int main()
{
    int R = 10;
    int N = 500;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < R; i++)
    {
        std::vector<std::thread> threads;
        for (int i = 0; i < N; i++)
        {
            std::thread t(&func, i);
            threads.push_back(std::move(t));
        }

        for (int i = 0; i < N; i++)
        {
            threads[(size_t)i].join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    milliseconds duration = std::chrono::duration_cast<milliseconds>(end - start);

    long T = duration.count();

    std::cout << std::fixed;
    std::cout << std::setprecision(1);
    std::cout << "dispatched " << R << " x " << N << " rpcs in " << T << "ms" << std::endl;
    std::cout << "rate:\t" << 1000 * (double)R * (double)N / (double)T << " rpcs/s" << std::endl;
    std::cout << "each rpc request is " << sizeof(basic_request) << "B" << std::endl;
    std::cout << "each rpc reply is " << sizeof(basic_reply) << "B" << std::endl;
    return 0;
}