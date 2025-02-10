#include <iostream>
#include <regex>

static bool isIpValid(const std::string &ip)
{
    const std::regex ipv4Pattern(
        R"(^([0-9]{1,3}\.){3}[0-9]{1,3}$)");

    if (!std::regex_match(ip, ipv4Pattern))
        return false;

    size_t start = 0;
    size_t end = ip.find('.');
    while (end != std::string::npos)
    {
        std::string octet = ip.substr(start, end - start);
        if (std::stoi(octet) < 0 || std::stoi(octet) > 255)
        {
            return false;
        }
        start = end + 1;
        end = ip.find('.', start);
    };

    std::string lastOctet = ip.substr(start);
    if (std::stoi(lastOctet) < 0 || std::stoi(lastOctet) > 255)
        return false;

    return true;
};

static bool isPortValid(const std::string &input)
{
    try
    {
        int port = std::stoi(input);
        return port >= 1025 && port <= 65535;
    }
    catch (const std::exception error)
    {
        return false;
    }
};