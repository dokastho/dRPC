#include "drpc.h"
#include <string>
#include <sys/socket.h>
#include <netdb.h>      // gethostbyname(), struct hostent
#include <netinet/in.h> // struct sockaddr_in
#include <string.h>
#include <thread>

drpc_server::drpc_server(drpc_host &host_args) : my_host(host_args) {}

void drpc_server::publish_endpoint(std::string func_name, void *func_ptr)
{
    endpoints[func_name] = func_ptr;
}

int drpc_server::run_server()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = my_host.port;
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
    listen(sockfd, 10);

    int connectionfd = accept(sockfd, 0, 0);
    parse_rpc(connectionfd);

    return 0;
}

void drpc_server::parse_rpc(int sockfd)
{
    drpc_msg m;

    // recv RPC
    // target function
    sock_lock.lock();
    {
        int len;
        recv(sockfd, &len, sizeof(int), MSG_WAITALL);
        recv(sockfd, &m.target, len, MSG_WAITALL);
    }
    // request args
    {
        recv(sockfd, &m.req->len, sizeof(int), MSG_WAITALL);
        recv(sockfd, &m.req->args, m.req->len, MSG_WAITALL);
    }
    // reply
    {
        recv(sockfd, &m.rep->len, sizeof(int), MSG_WAITALL);
        recv(sockfd, &m.rep->args, m.rep->len, MSG_WAITALL);
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

void drpc_server::stub(drpc_msg &m, int sockfd)
{
    void (*target_func)(void*, drpc_msg&) = (void (*)(void*, drpc_msg&))endpoints[m.target];
    if (target_func == nullptr)
    {
        // ignore
        return;
    }
    target_func(srv_ptr, m);
    // send RPC reply
    // reply
    {
        sock_lock.lock();
        send(sockfd, &m.rep->len, sizeof(int), MSG_WAITALL);
        send(sockfd, m.rep->args, m.rep->len, MSG_WAITALL);
        sock_lock.unlock();
    }
}