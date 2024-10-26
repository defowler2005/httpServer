#include "./httplib.h"
#include "./mime_types.h"
#include "./getlocalipv4.cpp"
#include "./writeLog.cpp"
#include "./isIpValid.cpp"
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

// args one variable => :{.

namespace fs = std::filesystem;

static std::string readFile(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
};

static void Send404(const httplib::Request &req, httplib::Response &res)
{
    std::string filePath = fs::current_path().string() + "/404.html";

    if (fs::exists(filePath))
        res.set_content(readFile(filePath), "text/html");
    else
        res.set_content("404: Page not found", "text/plain");
    res.status = 404;
    log("Client " + req.remote_addr + " requested: " + req.path + ", but the resource does not exist.");
};

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
            log("Invalid command: " + input + " commands include: exit\nhelp");
    }
};

static void startServer(httplib::Server &server, const std::string &ip, int port)
{
    server.listen(ip.c_str(), port);
};

int main(int argc, char *argv[])
{
    std::string ip = "127.0.0.1";
    int PORT = 80;
    std::string localIP = getLocalIPv4();
    httplib::Server server;

    if (argc >= 3)
    {
        log("Arguments were parsed, using them as IP and PORT...");

        if (!isIpValid(argv[1]))
        {
            log("You have entered an incorrectly structured IP. Structure example: '192.168.1.x.x'");
            return 0;
        }
        else
            ip = argv[1];

        int portInput = std::stoi(argv[2]);
        if (portInput < 1025 || portInput > 65535)
        {
            log("You have entered an invalid PORT. Port must range from a minimum of 1025 to a maximum of 65535");
            return 0;
        }
        else
            PORT = portInput;
    }
    else
    {
        if (!localIP.empty())
            ip = localIP;
        else
            log("Can't find computer's local IPv4. Using default IP (localhost:80)...");
    };

    server.Get("/", [&](const httplib::Request &req, httplib::Response &res)
               {
            std::string client_ip = req.remote_addr;
            std::string filePath = fs::current_path().string() + "/index.html";
            if (fs::exists(filePath)) {
                res.set_content(readFile(filePath), getMimeType(filePath));
                log("Client " + client_ip + " requested: /index.html");
            }
            else Send404(req, res); });

    server.Get("/test", [&](const httplib::Request &req, httplib::Response &res)
               {
            res.set_content("Test route tested!", "text/plain");
            log("Client " + req.remote_addr + " accessed the /test route."); });

    server.Get(".*", [&](const httplib::Request &req, httplib::Response &res)
               {
            std::string filePath = fs::current_path().string() + req.path;
            if (fs::exists(filePath)) {
                res.set_content(readFile(filePath), getMimeType(filePath));
                log("Client " + req.remote_addr + " requested: " + req.path);
            }
            else
                Send404(req, res); });

    log("The server is now running on http://" + ip + ":" + std::to_string(PORT));
    log("Enter command at any time!\n");
    std::string command = "netsh advfirewall firewall add rule name=\"CPPhttpServer\" dir=in action=allow protocol=TCP localport=" + std::to_string(PORT);
    system(command.c_str());
    log("Firewall rule added for port " + std::to_string(PORT));
    std::thread serverThread(startServer, std::ref(server), ip, PORT);
    std::thread inputThread(ConsoleReadKey);
    serverThread.join();
    inputThread.join();
    return 0;
};