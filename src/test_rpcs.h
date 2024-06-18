#ifndef TEST_RPCS
#define TEST_RPCS

#include <cstdint>
struct basic_request {
    char name[100];
    uint32_t seed;
};

struct basic_reply {
    int host;
    int status;    
};

#endif