using System;
using System.Threading;
using System.Threading.Tasks;
using RVTTClient;

namespace RVTTSimpleExample
{
    /// <summary>
    /// Simple console application example showing basic RVTT usage
    /// </summary>
    class Program
    {
        static async Task Main(string[] args)
        {
            Console.WriteLine("=== RVTT Simple Console Example ===\n");

            // Configuration
            string executablePath = args.Length > 0 ? args[0] : "./rvtt.exe";
            string modelPath = args.Length > 1 ? args[1] : "./models/ggml-base.bin";
            string language = args.Length > 2 ? args[2] : "ja";

            Console.WriteLine($"Executable: {executablePath}");
            Console.WriteLine($"Model: {modelPath}");
            Console.WriteLine($"Language: {language}\n");

            // Create transcriber instance
            using var transcriber = new RVTTTranscriber();

            // Subscribe to events
            transcriber.ProcessReady += (s, e) => 
            {
                Console.WriteLine("✓ RVTT process is ready");
            };

            transcriber.TranscriptionReceived += (s, e) => 
            {
                Console.WriteLine($"[{e.Timestamp:HH:mm:ss}] {e.Text}");
            };

            transcriber.ProcessTerminated += (s, e) => 
            {
                Console.WriteLine("✗ RVTT process terminated");
            };

            transcriber.TranscriptionComplete += (s, e) => 
            {
                Console.WriteLine("✓ Transcription complete (all buffered audio processed)");
            };

            // Start the process
            Console.WriteLine("Starting RVTT...");
            bool started = await transcriber.StartAsync(executablePath, modelPath, language, 30000);

            if (!started)
            {
                Console.WriteLine("Failed to start RVTT");
                return;
            }

            Console.WriteLine("✓ RVTT started successfully\n");

            // Start listening
            Console.WriteLine("Starting to listen...");
            transcriber.StartListening();
            Console.WriteLine("✓ Listening started\n");

            Console.WriteLine("Speak into your microphone. Press 'Q' to quit, 'P' to pause/resume.\n");
            Console.WriteLine("=== Transcription ===");

            bool isListening = true;

            // Main loop
            while (true)
            {
                if (Console.KeyAvailable)
                {
                    var key = Console.ReadKey(true);

                    if (key.Key == ConsoleKey.Q)
                    {
                        Console.WriteLine("\nQuitting...");
                        break;
                    }
                    else if (key.Key == ConsoleKey.P)
                    {
                        if (isListening)
                        {
                            transcriber.StopListening();
                            Console.WriteLine("\n[Paused - Waiting for transcription to complete...]");
                            
                            // Wait for transcription to complete (with 5 second timeout)
                            bool completed = transcriber.WaitForTranscriptionComplete(5000);
                            if (completed)
                            {
                                Console.WriteLine("[Paused - All transcription complete. Press 'P' to resume]");
                            }
                            else
                            {
                                Console.WriteLine("[Paused - Timeout waiting for completion. Press 'P' to resume]");
                            }
                            
                            isListening = false;
                        }
                        else
                        {
                            transcriber.StartListening();
                            Console.WriteLine("\n[Resumed - Listening...]");
                            isListening = true;
                        }
                    }
                }

                Thread.Sleep(100);
            }

            // Cleanup happens automatically via using statement
            Console.WriteLine("Goodbye!");
        }
    }
}

/*
 * To compile and run this example:
 * 
 * 1. Create a new console project:
 *    dotnet new console -n RVTTSimpleExample
 * 
 * 2. Copy this file and RVTTClient.cs to the project directory
 * 
 * 3. Update the .csproj to include both files:
 *    <ItemGroup>
 *      <Compile Include="RVTTClient.cs" />
 *      <Compile Include="SimpleConsoleExample.cs" />
 *    </ItemGroup>
 * 
 * 4. Build:
 *    dotnet build
 * 
 * 5. Run:
 *    dotnet run [executable_path] [model_path] [language]
 * 
 * Example:
 *    dotnet run ./rvtt.exe ./models/ggml-base.bin ja
 */

