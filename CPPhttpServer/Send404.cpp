#include "./httplib.h"
#include "./writeLog.cpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

namespace CPPhttpServer
{
    static void Send404(const httplib::Request& req, httplib::Response& res) {
        std::string filePath = std::filesystem::current_path().string() + "\\404.html";
        std::string content;

        if (std::filesystem::exists(filePath)) {
            std::ifstream file(filePath);
            content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        }
        else {
            content = R"(
<!DOCTYPE html>
<html lang='en-AU'>
   <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>404 - Resource Not Found</title>
      <meta name="color-scheme" content="light dark">
      <!-- Favicon not guaranteed to exist. -->
      <link rel='icon' type='image/x-icon' href='./favicon.ico'>
   </head>
   <body style="
      font-family: 'Arial', sans-serif; 
      margin: 0; 
      padding: 0; 
      display: flex; 
      justify-content: center; 
      align-items: center; 
      height: 100vh; 
      background-color: #f4f4f4;">
      <div style="text-align: center; color: #333;">
         <h1 style="font-size: 96px; color: #ff4c4c; margin: 0;">404</h1>
         <h2 style="font-size: 24px; margin: 0; color: #333;">Ouch! The resource was not found.</h2>
         <p style="font-size: 18px; color: #666; margin-bottom: 20px;">
            The resource you are looking for was either missing or was moved, likely removed.
         </p>
         <a href='/' style="
            font-size: 16px; 
            text-decoration: none; 
            color: #2196F3; 
            border: 2px solid #2196F3; 
            padding: 10px 20px; 
            border-radius: 5px; 
            transition: background-color 0.3s ease;" 
            onmouseover="this.style.backgroundColor='#2196F3'; this.style.color='white';" 
            onmouseout="this.style.backgroundColor='transparent'; this.style.color='#2196F3';">
         Go back to the homepage
         </a>
      </div>
   </body>
</html>
)";
        };

        res.status = 404;
        res.set_content(content, "text/html");
        log("Client requested a resource that does not exist: " + req.path);
    };
};