#include "Server.h"
#include <chrono>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BACKLOG 5
#define BUFFER_SIZE 16000

CServer::CServer() : _move()
{
    _txim = cv::Mat::zeros(10, 10, CV_8UC3);
}

CServer::~CServer()
{
    stop();
}

void CServer::start(int port)
{
    _server_exit = false;
    _server_thread = std::thread(&CServer::server_thread, this, port);
    _command_thread = std::thread(&CServer::process_commands, this);
}

void CServer::stop()
{
    _server_exit = true;
    if (_server_thread.joinable()) _server_thread.join();
    if (_command_thread.joinable()) _command_thread.join();
    _move.Stop();
}

bool CServer::setblocking(int fd, bool blocking)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0);
}

void CServer::server_thread(int port)
{
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = { 0 };

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket failed" << std::endl;
        return;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Setsockopt failed" << std::endl;
        close(server_fd);
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return;
    }

    // Listen
    if (listen(server_fd, BACKLOG) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return;
    }

    std::cout << "Server started on port " << port << std::endl;

    while (!_server_exit) {
        // Accept connection
        if ((client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            if (errno != EWOULDBLOCK) {
                std::cerr << "Accept failed" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        setblocking(client_fd, false);
        std::cout << "New client connected" << std::endl;

        // Handle client
        while (!_server_exit) {
            int bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);

            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::string cmd(buffer);

                _rx_mutex.lock();
                _cmd_list.push_back(cmd);
                _rx_mutex.unlock();
            }
            else if (bytes_read == 0) {
                // Client disconnected
                break;
            }
            else if (errno != EWOULDBLOCK) {
                // Error occurred
                break;
            }

            // Send any pending data
            _tx_mutex.lock();
            for (const auto& msg : _send_list) {
                send(client_fd, msg.c_str(), msg.length(), 0);
            }
            _send_list.clear();
            _tx_mutex.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        close(client_fd);
        std::cout << "Client disconnected" << std::endl;
    }

    close(server_fd);
}

void CServer::process_commands()
{
    while (!_server_exit) {
        std::vector<std::string> cmds;
        get_cmd(cmds);

        for (const auto& cmd : cmds) {
            execute_command(cmd);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void CServer::execute_command(const std::string& cmd)
{
    std::cout << "Executing: " << cmd << std::endl;

    if (cmd == "W") {
        _move.Forward(150);
        double start_time = cv::getTickCount();
        while ((cv::getTickCount() - start_time) / cv::getTickFrequency() < 0.5) {
            // Process other commands while waiting
            std::vector<std::string> cmds;
            get_cmd(cmds);
            for (const auto& c : cmds) {
                if (c == " ") {  // Allow immediate stop
                    _move.Stop();
                    return;
                }
            }
            cv::waitKey(10);  // Small delay to prevent CPU overload
        }
        _move.Stop();
    }
    else if (cmd == "S") {
        _move.Backward(150);
        double start_time = cv::getTickCount();
        while ((cv::getTickCount() - start_time) / cv::getTickFrequency() < 0.5) {
            // Process other commands while waiting
            std::vector<std::string> cmds;
            get_cmd(cmds);
            for (const auto& c : cmds) {
                if (c == " ") {  // Allow immediate stop
                    _move.Stop();
                    return;
                }
            }
            cv::waitKey(10);
        }
        _move.Stop();
    }
    else if (cmd == "A") {
        _move.TurnLeft(150);
        double start_time = cv::getTickCount();
        while ((cv::getTickCount() - start_time) / cv::getTickFrequency() < 0.5) {
            std::vector<std::string> cmds;
            get_cmd(cmds);
            for (const auto& c : cmds) {
                if (c == " ") {
                    _move.Stop();
                    return;
                }
            }
            cv::waitKey(10);
        }
        _move.Stop();
    }
    else if (cmd == "D") {
        _move.TurnRight(150);
        double start_time = cv::getTickCount();
        while ((cv::getTickCount() - start_time) / cv::getTickFrequency() < 0.5) {
            std::vector<std::string> cmds;
            get_cmd(cmds);
            for (const auto& c : cmds) {
                if (c == " ") {
                    _move.Stop();
                    return;
                }
            }
            cv::waitKey(10);
        }
        _move.Stop();
    }
    else if (cmd == "s") {
        _move.BoxUp();  // Immediate execution
    }
    else if (cmd == "t") {
        _move.BoxDown();  // Immediate execution
    }
    else if (cmd == " ") {
        _move.Stop();
        _move.BoxStop();
    }
}

void CServer::get_cmd(std::vector<std::string>& cmds)
{
    _rx_mutex.lock();
    cmds = _cmd_list;
    _cmd_list.clear();
    _rx_mutex.unlock();
}

void CServer::set_txim(cv::Mat& im)
{
    if (!im.empty()) {
        _image_mutex.lock();
        im.copyTo(_txim);
        _image_mutex.unlock();
    }
}