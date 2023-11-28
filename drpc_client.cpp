#include "drpc.h"
#include <string>
#include <netdb.h>      // gethostbyname(), struct hostent
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <string.h>

drpc_client::drpc_client() : timeout_val(DEFAULT_TIMEOUT) {}

drpc_client::drpc_client(const int timeout) : timeout_val(timeout) {}

int drpc_client::Call(drpc_host &srv, std::string funct, rpc_arg_wrapper *args, rpc_arg_wrapper *err)
{
    drpc_msg m{funct, args, err};
    int status = do_rpc(srv, m);
    if (status != 0)
    {
        perror("unable to complete RPC");
        return -1;
    }
    return 0;
}

int drpc_client::do_rpc(drpc_host &srv, drpc_msg &m)
{
    // connect to drpc server port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    struct hostent *host = gethostbyname(srv.hostname);
    if (host == nullptr)
    {
        fprintf(stderr, "%s: unknown host\n", srv.hostname);
        return 1;
    }
    memcpy(&(addr.sin_addr), host->h_addr, host->h_length);
    addr.sin_port = htons(srv.port);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket.");
        return 1;
    }

    struct timeval tv = {0, timeout_val * 1000}; // sleep for TIMEOUT ms

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)) < 0) {
        perror("Failed to set socket receive timeout.");
        return 1;
    }


    if (connect(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Error connecting stream socket");
        return 1;
    }

    // send RPC
    // target function
    {
        size_t len = m.target.size();
        send(sockfd, &len, sizeof(size_t), MSG_WAITALL | MSG_NOSIGNAL);
        send(sockfd, m.target.c_str(), len, MSG_WAITALL | MSG_NOSIGNAL);
    }
    // request args
    {
        send(sockfd, &m.req->len, sizeof(size_t), MSG_WAITALL | MSG_NOSIGNAL);
        send(sockfd, m.req->args, m.req->len, MSG_WAITALL | MSG_NOSIGNAL);
    }
    // reply
    {
        send(sockfd, &m.rep->len, sizeof(size_t), MSG_WAITALL | MSG_NOSIGNAL);
        send(sockfd, m.rep->args, m.rep->len, MSG_WAITALL | MSG_NOSIGNAL);
    }
    // checksum
    {
        // todo
    }

    // receive RPC reply
    // reply
    {
        ssize_t r = recv(sockfd, &m.rep->len, sizeof(size_t), MSG_WAITALL);
        // only call a second time if not timeout
        if (r != -1)
        {
            recv(sockfd, m.rep->args, m.rep->len, MSG_WAITALL);
        }
        
    }

    close(sockfd);

    return 0;
}