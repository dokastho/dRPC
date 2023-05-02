#include "drpc.h"
#include "test_rpcs.h"
#include <cassert>


int main()
{
    drpc_host h{"localhost", 8021};
    drpc_client c;

    basic_request breq{"basic test client", 0};
    basic_reply brep{0};
    rpc_arg_wrapper req{(void*)&breq, sizeof(basic_request)};
    rpc_arg_wrapper rep{(void*)&brep, sizeof(basic_reply)};

    c.Call(h, "foo", &req, &rep);
    assert(brep.status == 0xf);
    return 0;
}