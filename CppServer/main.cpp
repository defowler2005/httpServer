#include "./httplib.h";
#include "./json.h";
#include <chrono>;
#include <filesystem>;
#include <iostream>;

 ////////
 // TODO:
 // CONFIG OPTIONS: MIME-TYPES, BLACK LISTED FILE OR FOLDER PATHS AND OTHER SECURITY MESURES.
 ///////

namespace fs = std::filesystem;
using json = nlohmann::json;

static std::string getMimeType(const std::string &extension)
{
	static const std::unordered_map<std::string, std::string> mime_types = {
		{".html", "text/html"},
		{".htm", "text/html"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".json", "application/json"},
		{".txt", "text/plain"},
		{".csv", "text/csv"},
		{".xml", "application/xml"},
		{".jpeg", "image/jpeg"},
		{".png", "image/png"},
		{".ico", "image/x-icon"},
		{".gif", "image/gif"},
		{".bmp", "image/bmp"},
		{".svg", "image/svg+xml"},
		{".webp", "image/webp"},
		{".mp4", "video/mp4"},
		{".webm", "video/webm"},
		{".mp3", "audio/mp3"},
		{".wav", "audio/wav"},
		{".pdf", "application/pdf"},
		{".zip", "application/zip"},
		{".gz", "application/gzip"},
		{".woff", "font/woff"},
		{".woff2", "font/woff2"},
	};

	auto it = mime_types.find(extension);
	if (it != mime_types.end())
	return it -> second;
	else return "application/octet-stream";
};

static std::string readFile(const std::string &filePath)
{
	std::ifstream file(filePath, std::ios::in | std::ios::binary);

	if (!file)
	{
		return "";
	};
	std::ostringstream contents;
	contents << file.rdbuf();
	return contents.str();
};

static void log(const std::string& message) {
	auto now = std::chrono::system_clock::now();
	std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm;
#	ifdef _WIN32
	localtime_s(&now_tm, &now_time_t);
	#else
	localtime_r(&now_time_t, &now_tm);
	#endif
	std::ostringstream timeStream;
	timeStream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
	std::string timeString = timeStream.str();
	std::string formattedLog = "[" + timeString + "] - " + message;
	std::cout << formattedLog << std::endl;
	std::ofstream logFile("./CppServerLog.log", std::ios::app);
	logFile << formattedLog << std::endl;
	logFile.close();
};

int main()
{
	httplib::Server svr;
	std::string ip = "0.0.0.0";
	int port = 6432;
	std::string configFile = (fs::current_path() / "CppServerConfig.json").string();

	if (fs::exists(configFile)) {
		try
		{
			log("Config found!\n");
			std::ifstream dataFile("./CppServerConfig.json");
			json data = json::parse(dataFile);

			std::string newIp = data["ip"];
			std::string newPort = std::to_string(data["port"].get<int>());

			ip = newIp.empty() ? ip : newIp;
			port = newPort.empty() ? port : std::stoi(newPort);
		}
		catch (const std::exception& error)
		{
			log("Failed to parse JSON, is your IP valid? is your PORT valid?");
		}
	}
	else {
		log("Config not found! Writing to new CppServerConfig.json!\n");
		std::ofstream file("CppServerConfig.json");

		json configData = json::parse(R"(
			{   
				"_comment0" : "This is your current default configuration for the server.",
				"_comment1" : "You can change the IP and port to whatever you want to be, but make sure the values are valid. Otherwise the program will crash with no information or warning.",
				"ip": "0.0.0.0",
				"port": 6432
			}
		)");

		// Actually writing JSON data to the fuckin file!
		file << configData.dump(4);
	};

	svr.Get("/", [&](const httplib::Request &req, httplib::Response &res) // Root path (./index.html).
			{
			std::string client_ip = req.remote_addr;
			std::string filePath = (fs::current_path() / "index.html").string();

			if (fs::exists(filePath)) {
				res.set_content(readFile(filePath), "text/html");
				log("Client " + client_ip + " requested " + req.path + " (200 OK)");
			}
			else {
				log("Client " + client_ip + " requested " + req.path + " (404 Not Found)\n");
				res.status = 404;
				res.set_content("<h3 style='color: red;'>Main index.html file not found.</h1>", "text/html");
		} 
	}
);

	svr.Get(".*", [&](const httplib::Request &req, httplib::Response &res) { // Any file (*/*.*).
		std::string client_ip = req.remote_addr;
		std::string filePath = fs::current_path().string() + req.path;
		std::filesystem::path file(filePath);

		if (fs::exists(filePath))
		{
			res.set_content(readFile(filePath), getMimeType(file.extension().string()));
			log("Client " + client_ip + " requested " + req.path + " (200 OK)\n");
		}
		else
		{
			res.status = httplib::StatusCode::NotFound_404;
			std::string File404 = fs::current_path().string() + "/404.html";
			log("Client " + client_ip + " requested " + req.path + " (404 Not Found)\n");

			if (fs::exists(File404)) res.set_content(readFile(File404), "text/html");
			else res.set_content("<h3 style='color: red;'>The requested file " + req.path + " was not found on the server.</h3>", "text/html");
		}
	}
);

	try
	{
		log("Server listening on " + ip + ":" + std::to_string(port) + " All network interfaces.\n");
		svr.listen(ip, port);
	}
	catch (const std::exception &error)
	{
		std::cout << "An error occured while running the server program: " << error.what();
	}
};