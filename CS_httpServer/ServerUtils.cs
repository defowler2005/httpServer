using System.Globalization;
using System.Net;
using System.Net.Sockets;

namespace CS_httpServer
{
    internal class ServerUtils
    {
        private static readonly Dictionary<string, string> mimeTypes = new()
        {
            { ".html", "text/html" },
            { ".htm", "text/html" },
            { ".css", "text/css" },
            { ".js", "application/javascript" },
            { ".json", "application/json" },
            { ".png", "image/png" },
            { ".jpg", "image/jpeg" },
            { ".jpeg", "image/jpeg" },
            { ".gif", "image/gif" },
            { ".svg", "image/svg+xml" },
            { ".webp", "image/webp" },
            { ".ico", "image/x-icon" },
            { ".bmp", "image/bmp" },
            { ".tiff", "image/tiff" },
            { ".pdf", "application/pdf" },
            { ".zip", "application/zip" },
            { ".tar", "application/x-tar" },
            { ".gz", "application/gzip" },
            { ".xml", "application/xml" },
            { ".csv", "text/csv" },
            { ".txt", "text/plain" },
            { ".mp4", "video/mp4" },
            { ".mp3", "audio/mpeg" },
            { ".wav", "audio/wav" },
            { ".flac", "audio/flac" },
            { ".ogv", "video/ogg" },
            { ".ogg", "audio/ogg" },
            { ".webm", "video/webm" },
            { ".md", "text/markdown" },
            { ".doc", "application/msword" },
            { ".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
            { ".xls", "application/vnd.ms-excel" },
            { ".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
            { ".ppt", "application/vnd.ms-powerpoint" },
            { ".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation" },
            { ".heic", "image/heic" },
            { ".avi", "video/x-msvideo" },
            { ".mov", "video/quicktime" },
            { ".m4v", "video/x-m4v" },
            { ".eot", "application/vnd.ms-fontobject" },
            { ".ttf", "font/ttf" },
            { ".woff", "font/woff" },
            { ".woff2", "font/woff2" },
            { ".jsonld", "application/ld+json" },
            { ".wasm", "application/wasm" }
        };

        public static void Log(string message)
        {
            string timeString = DateTime.Now.ToString("dd-MM-yyyy hh:mm:ss tt", CultureInfo.InvariantCulture);
            Console.WriteLine($"[{timeString}] {message}");
        }

        public static string GetLocalIPv4()
        {
            try
            {
                string hostname = Dns.GetHostName();
                var hostEntry = Dns.GetHostEntry(hostname);

                for (int i = 0; i < hostEntry.AddressList.Length; i++)
                {
                    IPAddress? ip = hostEntry.AddressList[i];
                    if (ip != null && ip.AddressFamily == AddressFamily.InterNetwork)
                    {
                        return ip.ToString();
                    }
                }
            }
            catch (Exception ex)
            {
                Log($"An error occurred while getting the local IPv4:\n{ex}");
            }
            return "127.0.0.1";
        }

        public static void Send404(HttpListenerRequest req, HttpListenerResponse res)
        {
            string filePath = Path.Combine(Directory.GetCurrentDirectory(), "404.html");
            string content;
            res.ContentType = "text/html";

            if (File.Exists(filePath))
            content = File.ReadAllText(filePath);
            else
            content = @"
    <!DOCTYPE html>
    <html lang='en-AU'>
    <head>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>
        <title>404 - Resource Not Found</title>
        <!-- favicon.ico may or may not exist. -->
        <link type=""image/x-icon"" href=""./favicon.ico"">
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
            <h2 class='error-message'>Ouch! the resource was not found.</h2>
            <p class='error-description'>The resource you are looking for was either mising or was moved, likely removed.</p>
            <a href='/' class='back-home'>Go back to the homepage</a>
        </div>
    </body>
    </html>";

            res.StatusCode = (int)HttpStatusCode.NotFound;

            using (StreamWriter writer = new(res.OutputStream))
            writer.Write(content);

            if (req.Url != null)
            Log($"Client {req.RemoteEndPoint} requested {req.Url.AbsolutePath} but the resource does not exist.");
            else
            Log($"Client {req.RemoteEndPoint} requested a resource, but the URL is null.");
        }

        public static void ConsoleReadKey(HttpListener listener)
        {
            while (true)
            {
                string? input = Console.ReadLine();

                if (input != null)
                {
                    if (input == "exit" || input == "stop")
                    {
                        Log("Stopping the server...");
                        Thread.Sleep(1345);
                        listener.Stop();
                        Environment.Exit(0);
                    }
                    else
                    Log($"Invalid command: {input}. Commands include: exit\nhelp");
                }
                else
                Log("Received a null input. Please type a valid command.");
            }
        }

        public static string GetMimeType(string filePath)
        {
            string extension = Path.GetExtension(filePath);
            return mimeTypes.TryGetValue(extension, out string mimeType) ? mimeType : "application/octet-stream";
        }
    }
};