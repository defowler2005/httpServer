using System.Globalization;
using System.Net;

namespace CsServer
{
    class Program
    {
        private static readonly Dictionary<string, string> MimeTypes = new()
    {
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
        {".gz", "application/gzip"}
    };

        static string GetMimeType(string extension)
        {
            if (MimeTypes.TryGetValue(extension, out string? value))
                return value;
            return "application/octet-stream";
        }

        static string ReadFile(string filePath)
        {
            try
            {
                return File.ReadAllText(filePath);
            }
            catch
            {
                return string.Empty;
            }
        }

        static void Log(string message)
        {
            var time = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss", CultureInfo.InvariantCulture);
            Console.WriteLine($"[{time}] {message}");
        }

        static void Main()
        {
            string ip = "http://*:6432/";
            HttpListener listener = new();
            listener.Prefixes.Add(ip);

            try
            {
                listener.Start();
                Log($"Server listening on {ip} All network interfaces\n");

                while (true)
                {
                    HttpListenerContext context = listener.GetContext();
                    string clientIp = context.Request.RemoteEndPoint.ToString();
                    string requestedPath = context.Request.Url?.AbsolutePath ?? "/";

                    if (requestedPath == "/") // Root path.
                    {
                        string filePath = Path.Combine(Directory.GetCurrentDirectory(), "index.html");
                        if (File.Exists(filePath))
                        {
                            string content = ReadFile(filePath);
                            context.Response.ContentType = "text/html";
                            context.Response.StatusCode = 200;
                            using (StreamWriter writer = new(context.Response.OutputStream))
                            {
                                writer.Write(content);
                            }
                            Log($"Client {clientIp} requested {requestedPath} (200 OK)\n");
                        }
                        else
                        {
                            Log($"Client {clientIp} requested {requestedPath} (404 Not Found)\n");
                            context.Response.StatusCode = 404;
                            using StreamWriter writer = new(context.Response.OutputStream);
                            context.Response.ContentType = "text/html";
                            writer.Write("<h3 style='color: red;'>Main index.html file not found.</h3>");
                        }
                    }
                    else
                    { // Any file.
                        string filePath = Path.Combine(Directory.GetCurrentDirectory(), requestedPath.TrimStart('/'));
                        if (File.Exists(filePath))
                        {
                            string extension = Path.GetExtension(filePath);
                            string mimeType = GetMimeType(extension);
                            string content = ReadFile(filePath);
                            context.Response.ContentType = mimeType;
                            context.Response.StatusCode = 200;
                            using (StreamWriter writer = new(context.Response.OutputStream))
                            {
                                writer.Write(content);
                            }
                            Log($"Client {clientIp} requested {requestedPath} (200 OK)\n");
                        }
                        else
                        {
                            Log($"Client {clientIp} requested {requestedPath} (404 Not Found)\n");
                            context.Response.StatusCode = 404;
                            using StreamWriter writer = new(context.Response.OutputStream);
                            context.Response.ContentType = "text/html";
                            writer.Write($"<h3 style='color: red;'>The requested file {requestedPath} was not found on the server.</h3>");
                        }
                    }; context.Response.OutputStream.Close();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("An error occurred while running the server program: " + ex.Message);
            }
        }
    };
};