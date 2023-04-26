#ifndef DRPC_HEAD_H
#define DRPC_HEAD_H

#include <string>
#include <string.h>

struct rpc_arg_wrapper
{
    void *args;
    uint32_t len;
};

struct drpc_msg
{
    std::string target;
    rpc_arg_wrapper *req;
    rpc_arg_wrapper *rep;
};


#endif