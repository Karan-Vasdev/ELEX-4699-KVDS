#pragma once

#include "CMove.h"
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <opencv2/opencv.hpp>

#define PI4618  // Raspberry Pi flag

class CServer
{
public:
    CServer();
    ~CServer();

    void start(int port);
    void stop();
    void set_txim(cv::Mat& im);

private:
    bool setblocking(int fd, bool blocking);
    void server_thread(int port);
    void process_commands();
    void get_cmd(std::vector<std::string>& cmds);
    void execute_command(const std::string& cmd);

    // Motor control
    CMove _move;

    // Server state
    cv::Mat _txim;
    bool _server_exit = false;
    std::vector<std::string> _cmd_list;
    std::vector<std::string> _send_list;
    std::mutex _rx_mutex;
    std::mutex _tx_mutex;
    std::mutex _image_mutex;

    // Threads
    std::thread _server_thread;
    std::thread _command_thread;
};

