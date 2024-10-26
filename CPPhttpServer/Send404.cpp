#include "./httplib.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

namespace CPPhttpServer
{
    static void Send404(SOCKET clientSocket)
    {
        std::string filePath = std::filesystem::current_path().string() + "\\404.html"; // Adjust for Windows path
        std::string content;

        if (std::filesystem::exists(filePath))
        {
            std::ifstream file(filePath);
            content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        }
        else
        {
            content = R"(
<!DOCTYPE html>
<html lang='en-AU'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>404 - Resource Not Found</title>
    <link rel='icon' type='image/x-icon' href='./favicon.ico'>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            background-color: #f4f4f4;
        }
        .error-container {
            text-align: center;
            color: #333;
        }
        .error-code {
            font-size: 96px;
            color: #ff4c4c;
            margin: 0;
        }
        .error-message {
            font-size: 24px;
            margin: 0;
            color: #333;
        }
        .error-description {
            font-size: 18px;
            color: #666;
            margin-bottom: 20px;
        }
        .back-home {
            font-size: 16px;
            text-decoration: none;
            color: #2196F3;
            border: 2px solid #2196F3;
            padding: 10px 20px;
            border-radius: 5px;
            transition: background-color 0.3s ease;
        }
        .back-home:hover {
            background-color: #2196F3;
            color: white;
        }
    </style>
</head>
<body>
    <div class='error-container'>
        <h1 class='error-code'>404</h1>
        <h2 class='error-message'>Ouch! The resource was not found.</h2>
        <p class='error-description'>The resource you are looking for was either missing or was moved, likely removed.</p>
        <a href='/' class='back-home'>Go back to the homepage</a>
    </div>
</body>
</html>
)";
        }

        std::stringstream response;
        response << "HTTP/1.1 404 Not Found\r\n"
                 << "Content-Type: text/html\r\n"
                 << "Content-Length: " << content.size() << "\r\n"
                 << "Connection: close\r\n"
                 << "\r\n"
                 << content;

        send(clientSocket, response.str().c_str(), response.str().size(), 0);

        std::cout << "Client requested a resource that does not exist." << std::endl;
    }

}