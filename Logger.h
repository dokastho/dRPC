#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <sstream>
#include <fstream>
#include <mutex>
#include <iostream>

#include "drpc_msg.h"

#define sp ' '

// Class for logging atomically in correct format
class Logger
{
private:
    std::string log_file_name;
    std::ofstream log_file_fp;
    std::mutex m;

public:
    // opens log file for writing
    // closes when object is scoped out
    // ^ may have to change that if output is slow
    Logger(std::string log_file_name) : log_file_name(log_file_name)
    {
        // open file stream
        log_file_fp.open(log_file_name, std::ios::binary);
    }

    void write_line(rpc_arg_wrapper *req, rpc_arg_wrapper *rep)
    {
        m.lock();
        log_file_fp.write((char*)req->args, req->len);
        log_file_fp.write("\t\t", 2);
        log_file_fp.write((char*)rep->args, rep->len);
        log_file_fp.write("\n", 1);
        m.unlock();
        flush_log();
    }

    void flush_log()
    {
        m.lock();
        log_file_fp.flush();
        m.unlock();
    };

    ~Logger()
    {
        flush_log();
        log_file_fp.close();
    }
};
#endif