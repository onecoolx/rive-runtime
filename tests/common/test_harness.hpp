/*
 * Copyright 2024 Rive
 */

#pragma once

#include "common/tcp_client.hpp"
#include "common/queue.hpp"
#include "write_png_file.hpp"
#include <array>
#include <cassert>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>

struct ImageSaveArgs
{
    std::string name;
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> pixels;
    bool quit = false;
};

// Attempts to connect to the python server, and if successful, pipes stdout and
// stderr to the server. Also provides utilities for encoding & uploading PNGs, and
// notifying the server when the application crashes.
class TestHarness
{
public:
    static TestHarness& Instance();
    ~TestHarness() { shutdown(); }

    bool initialized() const { return m_initialized; }
    void init(const char* output, const char* toolName, size_t pngThreadCount);

    void setPNGCompression(PNGCompression compression) { m_pngCompression = compression; }

    void savePNG(ImageSaveArgs args)
    {
        assert(m_initialized);
        m_encodeQueue.push(std::move(args));
    }

    // Only returns true the on the first server request for a given name.
    // Prevents gms from running more than once in a multi-process execution.
    bool claimGMTest(const std::string&);

    void shutdown();

    void onApplicationCrash(const char* message);

private:
    TestHarness();

    // Adheres to a quick-and-dirty protocol for sending a PNG image back to the python harness.
    void sendImage(const std::string& remoteDestination,
                   uint32_t width,
                   uint32_t height,
                   uint8_t* imageDataRGBA);

    // Print a message on the server-side console.
    void sendConsoleMessage(const char* message, uint32_t messageLength);

    void shutdownStdioThread();

    // Forwards stdout and stderr to the server.
    static void MonitorStdIOThread(void* thisPtr)
    {
        reinterpret_cast<TestHarness*>(thisPtr)->monitorStdIOThread();
    }
    void monitorStdIOThread();

    // Encodes PNGS and sends them to the server.
    static void EncodePNGThread(void* thisPtr)
    {
        reinterpret_cast<TestHarness*>(thisPtr)->encodePNGThread();
    }
    void encodePNGThread();

    bool m_initialized = false;
    std::unique_ptr<TCPClient> m_primaryTCPClient;
    std::filesystem::path m_outputDir;

    // Forwarding stdout and stderr to the server.
    int m_savedStdout;
    int m_savedStderr;
    std::array<int, 2> m_stdioPipe = {0, 0};
    std::thread m_stdioThread;

    // PNG image encode queue.
    queue<ImageSaveArgs> m_encodeQueue{1};
    PNGCompression m_pngCompression = PNGCompression::compact;
    std::vector<std::thread> m_encodeThreads;
};