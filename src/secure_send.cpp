#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

#include "drpc.h"

extern SSL_CTX *create_context();
extern void configure_context(SSL_CTX *ctx);

void sslWrite(SSL *ssl, void *buf, size_t size)
{

    size_t writedTotal = 0;

    while (writedTotal < size) {
        int writedPart = SSL_write(ssl, static_cast<uint8_t*>(buf) + writedTotal, static_cast<int>(size - writedTotal   ));
        if (writedPart < 0) {
            std::string error = "SSL_write error = ";
            error += (std::to_string(writedPart) + ".");
            throw std::runtime_error(error);
        }
        writedTotal += writedPart;
    }
}

int secure_send(int sockfd, void* data_buf, size_t data_len)
{
    rpc_len_t len_sec{data_len, 0};
    len_sec.cksum = crc32(&len_sec, sizeof(rpc_len_t));

    rpc_len_t data_sec{len_sec.len, 0};
    data_sec.cksum = crc32(data_buf, data_len);
    data_sec.cksum = crc32(&data_sec, sizeof(rpc_len_t));

    SSL_CTX *ctx = create_context();
    configure_context(ctx);
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);

    sslWrite(ssl, &len_sec, sizeof(rpc_len_t));
    sslWrite(ssl, data_buf, len_sec.len);
    sslWrite(ssl, &data_sec, sizeof(rpc_len_t));

    return 0;
}