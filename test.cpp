#include "drpc.h"

void bar(int a, int b) {}

class Test
{
    public:
    Test()
    {
        drpc_host host{"aaa", 0};
        drpc_server s(host);
        s.publish_endpoint("foo", this->foo);
        // s.publish_endpoint("bar", bar);
    }

    static void foo(int a, int b) {}
};

int main()
{
    Test t();
    return 0;
}