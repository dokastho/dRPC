#ifndef DRPC_H
#define DRPC_H

#include <map>
#include <string>
#include <mutex>

#define SOCK_BUF_SIZE 100

struct rpc_arg_wrapper
{
    void *args;
    size_t len;
};

struct drpc_msg
{
    std::string target;
    rpc_arg_wrapper *req;
    rpc_arg_wrapper *rep;
};

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
    void *srv_ptr;
    std::mutex sock_lock, kill_lock;
    bool alive;

    void parse_rpc(int);

    void stub(drpc_msg, int);

public:
    drpc_server(drpc_host &, void *);

    void publish_endpoint(std::string, void *);

    int run_server();

    bool is_alive();

    void kill();
};

class drpc_client
{
private:
    int do_rpc(drpc_host &, drpc_msg &);

public:
    void Call(drpc_host &, std::string, rpc_arg_wrapper *, rpc_arg_wrapper *);
};

#endif