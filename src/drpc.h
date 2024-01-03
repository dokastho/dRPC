#ifndef DRPC_H
#define DRPC_H

#include <map>
#include <string>
#include <mutex>
#include <thread>
#include <string.h>

#include "Channel.h"

#define HOSTNAME_LEN 16
#define SOCK_BUF_SIZE 50
#define DEFAULT_TIMEOUT 500  // in ms

extern unsigned int crc32(const void *buf, size_t size);
extern int secure_send(int sockfd, void* data_buf, size_t data_len);
extern ssize_t secure_recv(int sockfd, void **data_buf);

struct rpc_arg_wrapper
{
    void *args;
    size_t len;
};

struct rpc_len_t
{
    size_t len;
    unsigned int cksum;
};

struct drpc_msg
{
    std::string target;
    rpc_arg_wrapper *req;
    rpc_arg_wrapper *rep;
};

struct drpc_host
{
    char hostname[HOSTNAME_LEN];
    short port;

    bool operator==(const drpc_host &rhs)
    {
        return port == rhs.port && (strcmp(hostname, rhs.hostname) == 0);
    }

    bool operator!=(const drpc_host &rhs)
    {
        return !(*this == rhs);
    }
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

    drpc_host get_host();
};

class drpc_client
{
private:
    int timeout_val;
    int do_rpc(drpc_host &, drpc_msg &);
    bool blocking = false;

public:
    drpc_client();
    drpc_client(const int);
    drpc_client(const int, bool);
    int Call(drpc_host &, std::string, rpc_arg_wrapper *, rpc_arg_wrapper *);
};

#endif