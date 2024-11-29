#include "./getlocalipv4.cpp"
#include "./httplib.h"
#include "./isIpValid.cpp"
#include "./mime_types.h"
#include "./writeLog.cpp"
#include "Send404.cpp"
#include <chrono>
#include <codecvt>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#endif

namespace fs = std::filesystem;
bool startLocalhost = false;

static std::wstring stringToWString(const std::string &str)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size);
    return wstr;
};

static std::string readFile(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
};

static void ConsoleReadKey()
{
    std::string input;

    while (true)
    {
        std::getline(std::cin, input);

        if (input == "exit")
        {
            log("Stopping the server...");
            Sleep(1345);
            exit(0);
        }
        else if (input == "about")
        {
            MessageBox(
                NULL,
                L"Desc",
                L"Title",
                MB_HELP | MB_OK
            );
        }
        else
            log("Invalid command: " + input + " commands include: exit, about");
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

    std::string userIp = (argc > 1) ? argv[1] : ip;
    std::string userPort = (argc > 2) ? argv[2] : std::to_string(PORT);

    if (argc >= 3)
    {
        if (isIpValid(userIp))
            ip = userIp;
        else
        {
            std::wstring msg = L"The IP address " + stringToWString(userIp) +
                               L" is unavailable to be used. Use your IPv4? " +
                               stringToWString(localIP) +
                               L"\n\nThe program will now start on the default IP configuration.";
            MessageBox(NULL, msg.c_str(), L"IP address unavailable", MB_OK | MB_ICONEXCLAMATION);
        }

        try
        {
            PORT = std::stoi(userPort);
        }
        catch (const std::exception &)
        {
            std::wstring msg = L"The PORT " + stringToWString(userPort) +
                               L" is unavailable to be used. Use your IPv4 and PORT 80?\n\n"
                               L"The program will now start on the default PORT configuration.";
            MessageBox(NULL, msg.c_str(), L"IP address unavailable", MB_OK | MB_ICONEXCLAMATION);
            PORT = 80;
        }
        log("Arguments were parsed, they will be used as IP and PORT.");
    }
    else
    {
        const int result = MessageBox(
            NULL,
            L"Click 'Yes' to start the server on 127.0.0.1, or No to start on your local IPv4. Both will use PORT 80",
            L"Server IP Type",
            MB_YESNO | MB_ICONQUESTION);

        startLocalhost = (result == IDYES);
        if (!startLocalhost && !localIP.empty())
            ip = localIP;
    };

    server.Get("/", [&](const httplib::Request &req, httplib::Response &res)
               {
        std::string client_ip = req.remote_addr;
        std::string filePath = fs::current_path().string() + "/index.html";
        if (fs::exists(filePath)) {
            res.set_content(readFile(filePath), getMimeType(filePath));
            log("Client " + client_ip + " requested: /index.html");
        }
            else
                CPPhttpServer::Send404(req, res); });

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
                CPPhttpServer::Send404(req, res); });

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