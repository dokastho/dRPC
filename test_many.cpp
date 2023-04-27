#include "drpc.h"
#include "test_rpcs.h"
#include <cassert>
#include <string>


int main()
{
    drpc_host h{"localhost", 8020};
    drpc_client c;

    int count = 100;

    for (int i = 0; i < count; i++)
    {
        std::string s = "many test client #" + std::to_string(i);
        basic_request breq;
        strcpy(breq.name, s.c_str());
        breq.seed = i;
        basic_reply brep{0};
        rpc_arg_wrapper req{(void*)&breq, sizeof(basic_request)};
        rpc_arg_wrapper rep{(void*)&brep, sizeof(basic_reply)};

        c.Call(h, "foo", &req, &rep);
        assert(brep.status == 0xf);
    }
    

    return 0;
}