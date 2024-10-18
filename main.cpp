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

namespace fs = std::filesystem;

static std::string readFile(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

static void consoleTextInput()
{
    std::string input;
    while (true)
    {
        std::getline(std::cin, input);
        if (input == "exit")
        {
            log("Stopping the server...");
            Sleep(1350);
            exit(0);
            break;
        }
    }
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
            log("You have entered and invalid IP.");
            return 0;
        }
        else
        {
            ip = argv[1];
        };

        if (std::stoi(argv[2]) < 1025 || std::stoi(argv[2]) > 65535)
        {
            log("You have entered and invalid PORT. Port must range from a minimum of 1025 and a maximum of 65535");
            return 0;
        }
        else
        {
            PORT = std::stoi(argv[2]);
        };
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
    }

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
                    res.set_content("404: File not found", "text/plain");
                }
                res.status = 404;
            }
        } });

    server.Get("/execute", [&](const httplib::Request &req, httplib::Response &res)
               {
        std::string client_ip = req.remote_addr;
        std::string requestPath = req.path;
        isRoute = true;
        res.set_content("Chrome executed!", "text/plain");
        system("C:/Users/defowler2005/Downloads/Application/chrome.exe");
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
    server.listen(ip.c_str(), PORT);

    std::thread inputThread(consoleTextInput);
    inputThread.join();
    return 0;
};