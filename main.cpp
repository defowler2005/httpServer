#include "./httplib.h"
#include "./mime_types.h"
#include "./getlocalipv4.cpp"
#include "./writeLog.cpp";
#include "./IPisValid.cpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <thread>
#include <wininet.h>
#include <windows.h>
#include <sys/types.h>

//404 function.
//args one variable.

namespace fs = std::filesystem;

static std::string readFile(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

static void ConsoleReadKey()
{
    std::string input;

    while (true)
    {
        std::getline(std::cin, input);

        if (input == "exit" || input == "stop")
        {
            log("Stopping the server...");
            Sleep(1345);
            exit(0);
        }
        else
            log("Invalid command: " + input + " commands include: exit\n");
    }
};

static void startServer(httplib::Server &server, const std::string &ip, int port)
{
    server.listen(ip.c_str(), port);
};

int main(int argc, char *argv[])
{
    std::string ip = "127.0.0.1";
    bool isRoute = false;
    int PORT = 80;
    std::string localIP = getLocalIPv4();

    httplib::Server server;

    if (argc >= 3)
    {
        log("Arguments were parsed, using them as IP and PORT...");

        if (IPisValid(argv[1]) == false)
        {
            log("You have entered an invalid IP.");
            return 0;
        }
        else
            ip = argv[1];

        if (std::stoi(argv[2]) < 1025 || std::stoi(argv[2]) > 65535)
        {
            log("You have entered an invalid PORT. Port must range from a minimum of 1025 to a maximum of 65535");
            return 0;
        }
        else
            PORT = std::stoi(argv[2]);
    }
    else
    {
        if (!localIP.empty())
        {
            ip = localIP;
        }
        else
        {
            std::cerr << "Can't find computer's local IPv4. Using default IP (localhost:80)..." << std::endl;
            return 0;
        }
    };

    server.Get("/", [&](const httplib::Request &req, httplib::Response &res)
               {
            std::string client_ip = req.remote_addr;
            std::string requestPath = req.path;
            isRoute = false;
            std::string filePath = fs::current_path().string() + "/index.html";
            if (fs::exists(filePath)) {
                res.set_content(readFile(filePath), getMimeType(filePath));
                log("Client " + client_ip + " requested: /index.html");
            }
            else {
                if (!isRoute) {
                    if (fs::exists(fs::current_path().string() + "/404.html")) {
                        res.set_content(readFile(fs::current_path().string() + "/404.html"), "text/html");
                    }
                    else {
                        res.set_content("404: Main /index.html not found", "text/plain");
                    }; res.status = 404;
                }
            } });

    server.Get("/test", [&](const httplib::Request &req, httplib::Response &res)
               {
                    std::string client_ip = req.remote_addr;
                    std::string requestPath = req.path;
                    isRoute = true;
                    res.set_content("Test route tested!", "text/plain");
                    log("Client " + client_ip + " accessed the " + requestPath + " route."); });

    server.Get(".*", [&](const httplib::Request &req, httplib::Response &res)
               {
                    std::string filePath = fs::current_path().string() + req.path;
                    std::string client_ip = req.remote_addr;
                    std::string requestPath = req.path;
                    if (fs::exists(filePath)) {
                        isRoute = false;
                        res.set_content(readFile(filePath), getMimeType(filePath));
                        log("Client " + client_ip + " requested: ." + requestPath);
                    }
                    else {
                        if (!isRoute) {
                            if (fs::exists(fs::current_path().string() + "/404.html")) {
                                res.set_content(readFile(fs::current_path().string() + "/404.html"), "text/html");
                            }
                            else {
                                res.set_content("404: File not found", "text/html");
                            }
                            res.status = 404;
                        }; log("Client " + client_ip + " requested: ." + requestPath + " but the resource does not exist.");
                    } });

    log("The server is now running on http://" + ip + ":" + std::to_string(PORT));
    log("Enter command at any time!\n");
    std::thread serverThread(startServer, std::ref(server), ip, PORT);
    std::thread inputThread(ConsoleReadKey);
    serverThread.join();
    inputThread.join();
    return 0;
};
