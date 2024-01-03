#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "drpc.h"

int secure_send(int sockfd, void* data_buf, size_t data_len)
{
    rpc_len_t len_sec{data_len, 0};
    len_sec.cksum = crc32(&len_sec, sizeof(rpc_len_t));

    rpc_len_t data_sec{len_sec.len, 0};
    data_sec.cksum = crc32(data_buf, data_len);

    send(sockfd, &len_sec, sizeof(rpc_len_t), MSG_WAITALL | MSG_NOSIGNAL);
    send(sockfd, data_buf, len_sec.len, MSG_WAITALL | MSG_NOSIGNAL);
    send(sockfd, &data_sec, sizeof(rpc_len_t), MSG_WAITALL | MSG_NOSIGNAL);
    return 0;
}