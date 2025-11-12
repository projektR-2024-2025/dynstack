using NetMQ;
using NetMQ.Sockets;
using System;
using System.Net;
using System.Net.Sockets;
using DynStacking.HotStorage.DataModel;
using System.Text;
using System.Net.WebSockets;
using System.Linq;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.IO;
using System.Diagnostics;
using System.ComponentModel;
using Microsoft.VisualBasic;

namespace DynStacking {
class Program {
	private static List<WebSocket> clients = new List<WebSocket>();
	private static String stanje;

	static async Task HandleWebserve(EventWaitHandle handle) {
		HttpListener listener = new HttpListener();
        listener.Prefixes.Add("http://+:8765/");
        listener.Start();
		Console.WriteLine("WebSocket server started. Listening on ws://localhost:8765/");
		while (true) {
			HttpListenerContext context = await listener.GetContextAsync();
			if (context.Request.IsWebSocketRequest) {
				HttpListenerWebSocketContext wsContext = await context.AcceptWebSocketAsync(null);
				WebSocket webSocket = wsContext.WebSocket;
				clients.Add(webSocket);
				handle.Set();
				if (string.IsNullOrEmpty(stanje)){
					byte[] buffer = new byte[4096];
					var sb = new StringBuilder();
					var segment = new ArraySegment<byte>(buffer);
					WebSocketReceiveResult result;
					result = await webSocket.ReceiveAsync(segment, CancellationToken.None);
					if (result.MessageType == WebSocketMessageType.Text) {
						sb.Append(Encoding.UTF8.GetString(buffer, 0, result.Count));
						if (result.EndOfMessage) {
							var message = sb.ToString();
							sb.Clear();
							if (string.Equals(message, "visual", StringComparison.OrdinalIgnoreCase)) {
								Process proc = new Process {
									StartInfo = new ProcessStartInfo {
										WorkingDirectory = "/app/simulation",
										FileName = "dotnet",
										Arguments = "run --project DynStack.SimulationRunner --sim HS --url tcp://0.0.0.0:2222 --id 658f9b28-6686-40d2-8800-611bd8466215 --settings Default",
										UseShellExecute = false,
										CreateNoWindow = true
									}
								};
								proc.Start();
								proc.BeginOutputReadLine();
								proc.BeginErrorReadLine();
								Console.WriteLine($"Started simulator (pid={proc.Id})");
								stanje = "visual";
								Console.WriteLine(stanje);
							}
							else {
								Process proc = new Process {
									StartInfo = new ProcessStartInfo {
										WorkingDirectory = "/app/simulation",
										FileName = "dotnet",
										Arguments = "run --project DynStack.SimulationRunner --sim HS --url tcp://0.0.0.0:8080 --id 658f9b28-6686-40d2-8800-611bd8466215 --settings Default --syncurl tcp://0.0.0.0:2222",
										UseShellExecute = false,
										CreateNoWindow = true
									}
								};
								proc.Start();
								proc.BeginOutputReadLine();
								proc.BeginErrorReadLine();
								Console.WriteLine($"Started simulator (pid={proc.Id})");
								stanje = "train";
								Console.WriteLine(stanje);
							}
						}
					}
					else {
						byte[] response = Encoding.UTF8.GetBytes($"{{\"stanje\" : \"{stanje}\"}}");
                		await webSocket.SendAsync(new ArraySegment<byte>(response), WebSocketMessageType.Text, true, CancellationToken.None);
					}
				}
				_ = HandleWebSocketConnection(webSocket);
			}
			else {
				string filePath = "./index.html";

				Console.WriteLine($"{filePath}");

				if (File.Exists(filePath))
				{
					string contentType = "text/html";
					if (Path.GetExtension(filePath).ToLower() == ".css")
						contentType = "text/css";
					else if (Path.GetExtension(filePath).ToLower() == ".js")
						contentType = "text/javascript";

					byte[] fileBytes = File.ReadAllBytes(filePath);
					context.Response.ContentType = contentType;
					context.Response.ContentLength64 = fileBytes.Length;
					context.Response.OutputStream.Write(fileBytes, 0, fileBytes.Length);
				}
				else
				{
					context.Response.StatusCode = 404;
				}
				context.Response.Close();
			}
		}
	}
	
	static async Task HandleWebSocketConnection(WebSocket webSocket) {
        byte[] buffer = new byte[1024];
        try {
            while (webSocket.State == WebSocketState.Open) {
                WebSocketReceiveResult result = await webSocket.ReceiveAsync(new ArraySegment<byte>(buffer), CancellationToken.None);
                if (result.MessageType == WebSocketMessageType.Close) {
					await webSocket.CloseAsync(WebSocketCloseStatus.NormalClosure, "Closed by client", CancellationToken.None);
					clients.Remove(webSocket);
                    return;
                }
            }
        }
        catch (Exception ex) {
            Console.WriteLine($"Error: {ex.Message}");
        }
        finally {
            if (webSocket.State == WebSocketState.Open)
                await webSocket.CloseAsync(WebSocketCloseStatus.InternalServerError, "Server error", CancellationToken.None);
        }
    }

	static async Task Main(string[] args) {
		if (args.Length < 2) {
			Console.WriteLine("Requires 3 arguments: SOCKET SIM_ID");
			return;
		}
		var socketAddr = args[0];
		var identity = new UTF8Encoding().GetBytes(args[1]);

		ManualResetEvent waitHandle = new ManualResetEvent(false);

		var webServerTask = HandleWebserve(waitHandle);
		Console.WriteLine("WebSocket server started in background");

		waitHandle.WaitOne();

		if(string.Equals(stanje, "visual", StringComparison.OrdinalIgnoreCase)) {
			using (var socket = new DealerSocket()) {
				socket.Options.Identity = identity;
				socket.Connect(socketAddr);
				Console.WriteLine("Connected");

				while (true) {
					Console.WriteLine("Waiting for request...");
					var request = socket.ReceiveMultipartBytes();
					Console.WriteLine("Incoming request");
					World world = World.Parser.ParseFrom(request[2]);
					if (clients.Any()) {
						foreach (WebSocket sock in clients) {
							byte[] response = Encoding.UTF8.GetBytes($"{world}");
							await sock.SendAsync(new ArraySegment<byte>(response), WebSocketMessageType.Text, true, CancellationToken.None);
						}
					}
				}
			}
		}

		waitHandle.Reset();
		waitHandle.WaitOne();
	}
}
}
