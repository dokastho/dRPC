#include "drpc.h"
#include "test_rpcs.h"
#include <cassert>


int main()
{
    drpc_host h{"localhost", 8020};
    drpc_client c;

    basic_reply* b = new basic_reply{0};
    rpc_arg_wrapper req{nullptr, 0};
    rpc_arg_wrapper rep{(void*)b, sizeof(basic_reply)};

    c.Call(h, "foo", &req, &rep);
    assert(b->status == 0xf);
    return 0;
}