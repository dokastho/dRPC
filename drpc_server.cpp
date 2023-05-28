#include "drpc.h"
#include <string>
#include <sys/socket.h>
#include <netdb.h>      // gethostbyname(), struct hostent
#include <netinet/in.h> // struct sockaddr_in
#include <string.h>
#include <thread>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

drpc_server::drpc_server(drpc_host &host_args, void *srv_ptr_arg) : my_host(host_args)
{
    srv_ptr = srv_ptr_arg;
    alive = true;
}

void drpc_server::publish_endpoint(std::string func_name, void *func_ptr)
{
    endpoints[func_name] = func_ptr;
}

bool drpc_server::is_alive()
{
    bool b;
    kill_lock.lock();
    b = alive;
    kill_lock.unlock();
    return b;
}

void drpc_server::kill()
{
    kill_lock.lock();
    alive = false;
    kill_lock.unlock();
}

int drpc_server::run_server()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(my_host.port);

    // start server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("creating socket");
        return 1;
    }

    // (2) Set the "reuse port" socket option
    int yesval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval)) == -1)
    {
        perror("socket options");
        return 1;
    }

    // (3b) Bind to the port.
    if (bind(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("port");
        return -1;
    }
    listen(sockfd, SOCK_BUF_SIZE);

    while (is_alive())
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        struct timeval tv = {0, 100000}; // sleep for 100 ms

        if (select(sockfd + 1, &readfds, NULL, NULL, &tv) < 0)
        {
            // error or timeout
            // noop
        }
        if (FD_ISSET(sockfd, &readfds))
        {
            int connectionfd = accept(sockfd, 0, 0);
            parse_rpc(connectionfd);
        }
        else
        {
            // the socket timedout
        }
    }

    return 0;
}

void drpc_server::parse_rpc(int sockfd)
{
    rpc_arg_wrapper *req = new rpc_arg_wrapper;
    rpc_arg_wrapper *rep = new rpc_arg_wrapper;
    drpc_msg m;
    m.req = req;
    m.rep = rep;

    // recv RPC
    // target function
    sock_lock.lock();
    {
        size_t len;
        recv(sockfd, &len, sizeof(size_t), MSG_WAITALL);
        char *buf = (char *)malloc(len);
        recv(sockfd, buf, len, MSG_WAITALL);
        m.target = std::string(buf, len);
        free(buf);
    }
    // request args
    {
        recv(sockfd, &m.req->len, sizeof(size_t), MSG_WAITALL);
        m.req->args = (void *)malloc(m.req->len);
        recv(sockfd, m.req->args, m.req->len, MSG_WAITALL);
    }
    // reply
    {
        recv(sockfd, &m.rep->len, sizeof(size_t), MSG_WAITALL);
        m.rep->args = (void *)malloc(m.rep->len);
        recv(sockfd, m.rep->args, m.rep->len, MSG_WAITALL);
    }
    // checksum
    {
        // todo
    }

    sock_lock.unlock();

    // call function
    std::thread t(&drpc_server::stub, this, m, sockfd);
    t.detach();
}

void drpc_server::stub(drpc_msg m, int sockfd)
{
    static void (*target_func)(void *, drpc_msg &) = nullptr;
    target_func = (void (*)(void *, drpc_msg &))endpoints[m.target];
    if (target_func == nullptr)
    {
        // ignore
        return;
    }
    target_func(srv_ptr, m);
    // send RPC reply
    // reply
    sock_lock.lock();
    try
    {
        send(sockfd, &m.rep->len, sizeof(size_t), MSG_WAITALL);
        send(sockfd, m.rep->args, m.rep->len, MSG_WAITALL);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    sock_lock.unlock();
    
    free(m.req->args);
    free(m.rep->args);
    m.req->args = nullptr;
    m.rep->args = nullptr;
    delete m.req;
    delete m.rep;
    m.rep = nullptr;
    m.req = nullptr;
}