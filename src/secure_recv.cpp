#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

#include "drpc.h"

extern SSL_CTX *create_context();
extern void configure_context(SSL_CTX *ctx);

ssize_t sslRead(SSL *ssl, void *buf, size_t size)
{
    size_t readTotal = 0;

    while (readTotal < size)
    {
        int readPart = SSL_read(ssl, static_cast<uint8_t*>(buf) + readTotal, static_cast<int>(size - readTotal));
        if (readPart < 0)
        {
            return -1;
        }
        readTotal += readPart;
    }

    return 0;
}

ssize_t secure_recv(int sockfd, void **data_buf)
{
    SSL_CTX *ctx = create_context();
    configure_context(ctx);
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);

    unsigned int expected_cksum;
    rpc_len_t len_sec{0, 0};
    if (sslRead(ssl, &len_sec, sizeof(rpc_len_t)) == -1)
        return -1;
    expected_cksum = len_sec.cksum;
    len_sec.cksum = 0;

    if (expected_cksum != crc32(&len_sec, sizeof(rpc_len_t)))
    {
        return -1;
    }
    size_t data_len = len_sec.len;

    bool data_buf_is_null = false;
    if (!*data_buf)
    {
        data_buf_is_null = true;
        *data_buf = malloc(data_len);
    }

    if (sslRead(ssl, *data_buf, data_len) == -1)
        return -1;

    // receive data cheksum bytes
    rpc_len_t data_sec{0, 0};
    if (sslRead(ssl, &data_sec, sizeof(rpc_len_t)) == -1)
        return -1;

    expected_cksum = data_sec.cksum;
    data_sec.cksum = crc32(*data_buf, data_len);
    data_sec.cksum = crc32(&data_sec, sizeof(rpc_len_t));

    // use data_len as a checksum proxy to validate the data checksum bytes
    if (data_sec.len != data_len || data_sec.cksum != expected_cksum)
    {
        if (data_buf_is_null)
        {
            free(*data_buf);
            *data_buf = nullptr;
        }

        return -1;
    }
    return data_len;
}