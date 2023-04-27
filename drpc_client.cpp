#include "drpc.h"
#include <string>
#include <sys/socket.h>
#include <netdb.h>      // gethostbyname(), struct hostent
#include <netinet/in.h> // struct sockaddr_in
#include <string.h>

void drpc_client::Call(drpc_host &srv, std::string funct, rpc_arg_wrapper *args, rpc_arg_wrapper *err)
{
    drpc_msg m{funct, args, err};
    int status = do_rpc(srv, m);
    if (status != 0)
    {
        perror("unable to complete RPC");
    }
}

int drpc_client::do_rpc(drpc_host &srv, drpc_msg &m)
{
    // connect to drpc server port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    struct hostent *host = gethostbyname(srv.hostname.c_str());
    if (host == nullptr)
    {
        fprintf(stderr, "%s: unknown host\n", srv.hostname.c_str());
        return 1;
    }
    memcpy(&(addr.sin_addr), host->h_addr, host->h_length);
    addr.sin_port = srv.port;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Error connecting stream socket");
        return 1;
    }

    // send RPC
    // target function
    {
        size_t len = m.target.size();
        send(sockfd, &len, sizeof(size_t), MSG_WAITALL);
        send(sockfd, m.target.c_str(), len, MSG_WAITALL);
    }
    // request args
    {
        send(sockfd, &m.req->len, sizeof(size_t), MSG_WAITALL);
        send(sockfd, m.req->args, m.req->len, MSG_WAITALL);
    }
    // reply
    {
        send(sockfd, &m.rep->len, sizeof(size_t), MSG_WAITALL);
        send(sockfd, m.rep->args, m.rep->len, MSG_WAITALL);
    }
    // checksum
    {
        // todo
    }

    // receive RPC reply
    // reply
    {
        recv(sockfd, &m.rep->len, sizeof(int), MSG_WAITALL);
        recv(sockfd, m.rep->args, m.rep->len, MSG_WAITALL);
    }

    return 0;
}