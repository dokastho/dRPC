#include <exception>
#include <string>
#include <sys/socket.h>
#include <netdb.h>      // gethostbyname(), struct hostent
#include <netinet/in.h> // struct sockaddr_in
#include <string.h>
#include <thread>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

#include "drpc.h"
#include "Channel.h"

drpc_server::drpc_server(drpc_host &host_args, void *srv_ptr_arg) : my_host(host_args)
{
    srv_ptr = srv_ptr_arg;
    alive = true;
}

drpc_server::~drpc_server()
{
    this->kill();
    // join if running thread not null
    // is null if server wasn't started
    if (running)
    {
        running->join();
        delete running;
    }
    for (auto it = threads.begin(); it != threads.end(); it++)
    {
        it->second.join();
    }
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

void drpc_server::start()
{
    // make it run on another thread, and join in dtor to prevent invalid reads
    running = new std::thread(&drpc_server::run_server, this);
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

    // close socket to allow reuse
    close(sockfd);
    return 0;
}

void sends(int sockfd, void *buf, size_t len)
{
    size_t sent = 0;
    do
    {
        ssize_t n = send(sockfd, (uint8_t *)buf + sent, len - sent, MSG_NOSIGNAL);
        if (n == -1)
        {
            perror("Error sending on stream socket");
            return;
        }
        sent += n;
    } while (sent < len);
}

void recvs(int sockfd, void *buf, size_t len)
{
    size_t recvd = 0;
    do
    {
        ssize_t n = recv(sockfd, (uint8_t *)buf + recvd, len - recvd, 0);
        if (n == -1)
        {
            perror("Error recving on stream socket");
            exit(1);
        }
        recvd += n;
    } while (recvd < len);
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
        recvs(sockfd, &len, sizeof(size_t));
        char *buf = (char *)malloc(len);
        recvs(sockfd, buf, len);
        m.target = std::string(buf, len);
        free(buf);
    }
    // request args
    {
        recvs(sockfd, &m.req->len, sizeof(size_t));
        m.req->args = (void *)malloc(m.req->len);
        recvs(sockfd, m.req->args, m.req->len);
    }
    // reply
    {
        recvs(sockfd, &m.rep->len, sizeof(size_t));
        m.rep->args = (void *)malloc(m.rep->len);
        recvs(sockfd, m.rep->args, m.rep->len);
    }
    // checksum
    {
        // todo
    }

    sock_lock.unlock();

    // if all threads are busy, then wait for one to complete
    while (threads.size() >= SOCK_BUF_SIZE)
    {
        // join thread if one is finished or wait if not
        int complete_thread_id = finished_threads.get();
        threads[complete_thread_id].join();
        threads.erase(complete_thread_id);
    }


    // call function and save random thread id
    int t_id = rand();
    std::thread t(&drpc_server::stub, this, m, sockfd, t_id);
    threads[t_id] = std::move(t);
}

void drpc_server::stub(drpc_msg m, int sockfd, int my_id)
{
    static void (*target_func)(void *, drpc_msg &) = nullptr;
    target_func = (void (*)(void *, drpc_msg &))endpoints[m.target];
    if (target_func != nullptr)
    {
        target_func(srv_ptr, m);
        // send RPC reply
        // reply
        sock_lock.lock();
        sends(sockfd, &m.rep->len, sizeof(size_t));
        sends(sockfd, m.rep->args, m.rep->len);
        sock_lock.unlock();
    }

    free(m.req->args);
    free(m.rep->args);
    m.req->args = nullptr;
    m.rep->args = nullptr;
    delete m.req;
    delete m.rep;
    m.rep = nullptr;
    m.req = nullptr;

    // announce thread is about to be complete
    finished_threads.add(my_id);
}