#pragma once

// A simple reader/writer class that provides support for reading and writing
// files on disk. Provides synchronous and asynchronous methods.
class BasicReaderWriter
{
private:
    winrt::Windows::Storage::StorageFolder m_location{ nullptr };

public:
    BasicReaderWriter();
    BasicReaderWriter(
        winrt::Windows::Storage::StorageFolder const& folder
    );

    std::vector<byte> ReadData(
        std::wstring const& filename
    );

    std::future<std::vector<byte>> ReadDataAsync(
        std::wstring const& filename
    );

    uint32_t WriteData(
        std::wstring const& filename,
        std::vector<byte> fileData
    );

    winrt::Windows::Foundation::IAsyncAction WriteDataAsync(
        std::wstring const& filename,
        std::vector<byte> fileData
    );
};