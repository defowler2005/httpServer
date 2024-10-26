#include <chrono>;
#include <iostream>;

static void log(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);
    std::ostringstream timeStream;
    timeStream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    std::string timeString = timeStream.str();
    std::cout << "[" << timeString << "] " << message << std::endl;
};