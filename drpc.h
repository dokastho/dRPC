#ifndef DRPC_H
#define DRPC_H

#include <map>
#include <string>
#include <mutex>
#include "drpc_msg.h"

struct drpc_host
{
    std::string hostname;
    short port;
};

class drpc_server
{
private:
    std::map<std::string, void *> endpoints;
    drpc_host my_host;
    void* srv_ptr;
    std::mutex sock_lock;
    
    void parse_rpc(int);

    void stub(drpc_msg, int);

public:
    drpc_server(drpc_host&);

    void publish_endpoint(std::string, void *);

    int run_server();
};

class drpc_client
{
private:
    int do_rpc(drpc_host &, drpc_msg &);

public:
    void Call(drpc_host &, std::string, rpc_arg_wrapper *, rpc_arg_wrapper *);
};

#endif