using System.Net;
using System.Diagnostics;
using static CS_httpServer.ServerUtils;

namespace CS_httpServer
{
    public static class Program
    {
        static void Main(string[] args)
        {
            string ip = "127.0.0.1";
            int port = 80;
            string localIP = GetLocalIPv4();
            string CurrentExecutableName = System.AppDomain.CurrentDomain.FriendlyName;

            if (args.Length >= 1)
            {
                Log("Arguments were parsed, using them as IP and PORT...");

                if (!IPAddress.TryParse(args[0], out _))
                {
                    Log("You have entered an incorrectly structured IP. Structure example: '192.168.1.x'");
                }; ip = args[0];

                if (int.TryParse(args[1], out int portInput) && portInput >= 1025 && portInput <= 65535) port = portInput;
                else
                {
                    Log("You have entered an invalid PORT. Port must range from a minimum of 1025 to a maximum of 65535");
                    return;
                }
            }
            else { if (!string.IsNullOrEmpty(localIP)) ip = localIP; };

            using var server = new HttpListener();
            server.Prefixes.Add($"http://{ip}:{port}/");
            server.Start();
            Log($"The server is now running on http://{ip}:{port}");
            Log("Enter command at any time!");

            try
            {
                Process.Start(new ProcessStartInfo("netsh", $"advfirewall firewall add rule name=\"{CurrentExecutableName}\" dir=in action=allow protocol=TCP localport={port}")
                {
                    UseShellExecute = true,
                    Verb = "runas"
                });
                Log($"Firewall rule added for port {port}");
            }
            catch (Exception ex)
            {
                Log($"Failed to add firewall rule: {ex.Message}");
            }; Task.WaitAll(StartServer(server), Task.Run(() => ConsoleReadKey(server)));
        }

        static async Task StartServer(HttpListener server)
        {
            while (true)
            {
                var context = await server.GetContextAsync();
                var req = context.Request;
                var res = context.Response;
                string client_ip = req.RemoteEndPoint.ToString();
                string requestedUrl = req.Url?.AbsolutePath ?? string.Empty;

                try
                {
                    switch (requestedUrl)
                    {
                        case "/": case "/index.html":
                            string indexFile = Path.Combine(Directory.GetCurrentDirectory(), "index.html");
                            if (File.Exists(indexFile))
                            {
                                res.ContentType = "text/html";
                                Log($"Client {client_ip} requested index.html");
                                using var writer = new StreamWriter(res.OutputStream);
                                await writer.WriteAsync(File.ReadAllText(indexFile));
                            }
                            else Send404(req, res);
                            break;

                        case "/test":
                            res.ContentType = "text/plain";
                            Log($"Client {client_ip} requested /test");
                            using (var writer = new StreamWriter(res.OutputStream))
                            {
                                await writer.WriteAsync("This is a test response.");
                                Log($"Client {client_ip} accessed the /test route.");
                            }; break;

                        default:
                            string filePath = Path.Combine(Directory.GetCurrentDirectory(), requestedUrl.TrimStart('/'));
                            if (File.Exists(filePath))
                            {
                                res.ContentType = GetMimeType(filePath);
                                using var writer = new StreamWriter(res.OutputStream);
                                await writer.WriteAsync(File.ReadAllText(filePath));
                                Log($"Client {client_ip} requested {requestedUrl}");
                            }
                            else Send404(req, res);
                            break;
                    }
                }
                catch (Exception ex) { 
                    res.Close();
                    Log($"The server was closed as a result of the following error: {ex}");
                }
            }
        }
    }
};