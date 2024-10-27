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

namespace fs = std::filesystem;
bool startLocalhost = false;

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
    try
    {
        if (startLocalhost)
        {
            server.listen("127.0.0.1", port);
        }
        else
        {
            server.listen(ip.c_str(), port);
        }
    }
    catch (const std::exception &e)
    {
        log("Failed to start the server on port " + std::to_string(port) + ": " + e.what());
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "That port " << port << " is currently already in use." << std::endl;
        exit(1);
    }
};

int main(int argc, char *argv[])
{
    std::string ip = "127.0.0.1";
    int PORT = 80;
    std::string localIP = getLocalIPv4();
    httplib::Server server;

    const int result = MessageBox(
        NULL,
        L"Click 'Yes' to start the server on 127.0.0.1, or No to start on your local IPv4.",
        L"Server IP Type",
        MB_YESNO | MB_ICONQUESTION);

    startLocalhost = (result == IDYES);
    if (!startLocalhost && !localIP.empty())
        ip = localIP;

    server.Get("/", [&](const httplib::Request &req, httplib::Response &res)
               {
            std::string client_ip = req.remote_addr;
            std::string filePath = fs::current_path().string() + "/index.html";
            if (fs::exists(filePath)) {
                res.set_content(readFile(filePath), getMimeType(filePath));
                log("Client " + client_ip + " requested: /index.html");
            }
            else
                Send404(req, res); });

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