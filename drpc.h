#ifndef DRPC_H
#define DRPC_H

#include <map>
#include <string>
#include <mutex>
#include <thread>

#include "Channel.h"

#define SOCK_BUF_SIZE 50
#define DEFAULT_TIMEOUT 500  // in ms

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
    std::thread *running;
    std::mutex sock_lock, kill_lock;
    std::map<int, std::thread> threads;
    Channel<int> finished_threads;
    bool alive;

    void parse_rpc(int);

    void stub(drpc_msg, int, int);

    int run_server();

public:
    drpc_server(drpc_host &, void *);

    ~drpc_server();

    void publish_endpoint(std::string, void *);

    void start();

    bool is_alive();

    void kill();
};

class drpc_client
{
private:
    int timeout_val;
    int do_rpc(drpc_host &, drpc_msg &);

public:
    drpc_client();
    drpc_client(const int);
    int Call(drpc_host &, std::string, rpc_arg_wrapper *, rpc_arg_wrapper *);
};

#endif