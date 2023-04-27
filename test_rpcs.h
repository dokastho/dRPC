#ifndef TEST_RPCS
#define TEST_RPCS

struct basic_request {
    char name[100];
};

struct basic_reply {
    int status;    
};

#endif