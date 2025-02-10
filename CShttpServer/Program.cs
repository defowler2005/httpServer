using System.Net;
using static CSHttpServer.ServerUtils;

namespace CSHttpServer
{
    public static class Program
    {
        static void Main(string[] args)
        {
            string ip = "127.0.0.1";
            int port = 80;
            string localIP = GetLocalIPv4();
            string CurrentExecutableName = AppDomain.CurrentDomain.FriendlyName;

            if (args.Length >= 1)
            {
                Log("Arguments were parsed, using them as IP and PORT...");

                if (!IPAddress.TryParse(args[0], out _))
                {
                    Log("You have entered an incorrectly structured IP. Structure example: '192.168.1.x'");
                }
                ip = args[0];

                if (int.TryParse(args[1], out int portInput) && portInput >= 1025 && portInput <= 65535)
                    port = portInput;
                else
                {
                    Log("You have entered an invalid PORT. Port must range from a minimum of 1025 to a maximum of 65535");
                    return;
                }
            }
            else if (!string.IsNullOrEmpty(localIP)) ip = localIP;

            using var server = new HttpListener();
            server.Prefixes.Add($"http://{ip}:{port}/");
            server.Start();
            Log($"The server is now running on http://{ip}:{port}");
            Log("Enter command at any time!");
            Task.WhenAll(StartServer(server), Task.Run(() => ConsoleReadKey(server))).Wait();
        }

        static async Task StartServer(HttpListener server)
        {
            while (server.IsListening)
            {
                var context = await server.GetContextAsync();
                _ = Task.Run(() => HandleRequest(context));
            };
        }

        static async Task HandleRequest(HttpListenerContext context)
        {
            var req = context.Request;
            var res = context.Response;
            string client_ip = req.RemoteEndPoint.ToString();
            string requestedUrl = req.Url?.AbsolutePath ?? string.Empty;

            try
            {
                string filePath = Path.Combine(Directory.GetCurrentDirectory(), requestedUrl.TrimStart('/'));

                if (requestedUrl == "/" || requestedUrl == "/index.html")
                {
                    filePath = Path.Combine(Directory.GetCurrentDirectory(), "index.html");
                }

                if (File.Exists(filePath))
                {
                    res.ContentType = GetMimeType(filePath);
                    byte[] fileBytes = await File.ReadAllBytesAsync(filePath);
                    res.ContentLength64 = fileBytes.Length;

                    await res.OutputStream.WriteAsync(fileBytes);
                    Log($"Client {client_ip} requested {requestedUrl}");
                }
                else
                {
                    Send404(req, res);
                }
            }
            catch (Exception ex)
            {
                Log($"Request handling error: {ex}");
            }
            finally
            {
                res.Close();
            }
        }
    }
};