#include "drpc.h"
#include <iostream>

void bar(int a, int b)
{
    a += b;
    b += a;
}

class Test
{
public:
    Test()
    {
        drpc_host host{"aaa", 0};
        drpc_server s(host);
        s.publish_endpoint("foo", (void *)this->foo);
    }

    static void foo(int a, int b)
    {
        a += b;
        b += a;
    }
};

int main()
{
    Test t;
    return 0;
}