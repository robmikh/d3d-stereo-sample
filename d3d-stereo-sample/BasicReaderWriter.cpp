#include "pch.h"
#include "BasicReaderWriter.h"

namespace winrt
{
    using namespace Windows::Storage;
    using namespace Windows::Storage::FileProperties;
    using namespace Windows::Storage::Streams;
    using namespace Windows::Foundation;
    using namespace Windows::ApplicationModel;
}

BasicReaderWriter::BasicReaderWriter()
{
    m_location = winrt::Package::Current().InstalledLocation();
}

BasicReaderWriter::BasicReaderWriter(
    winrt::Windows::Storage::StorageFolder const& folder)
{
    m_location = folder;
    std::wstring path(m_location.Path());
    if (path.length() == 0)
    {
        // Applications are not permitted to access certain
        // folders, such as the Documents folder, using this
        // code path.  In such cases, the Path property for
        // the folder will be an empty string.
        throw winrt::hresult_error(E_UNEXPECTED);
    }
}

std::vector<byte> BasicReaderWriter::ReadData(
    std::wstring const& filename)
{
    CREATEFILE2_EXTENDED_PARAMETERS extendedParams = { 0 };
    extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
    extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
    extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
    extendedParams.lpSecurityAttributes = nullptr;
    extendedParams.hTemplateFile = nullptr;

    wil::unique_hfile file(
        CreateFile2(
            filename.data(),
            GENERIC_READ,
            FILE_SHARE_READ,
            OPEN_EXISTING,
            &extendedParams
        )
    );
    if (file.get() == INVALID_HANDLE_VALUE)
    {
        throw winrt::hresult_error(E_UNEXPECTED);
    }

    FILE_STANDARD_INFO fileInfo = {};
    if (!GetFileInformationByHandleEx(
        file.get(),
        FileStandardInfo,
        &fileInfo,
        sizeof(fileInfo)
    ))
    {
        throw winrt::hresult_error(E_UNEXPECTED);
    }

    if (fileInfo.EndOfFile.HighPart != 0)
    {
        throw winrt::hresult_error(E_OUTOFMEMORY);
    }

    std::vector<byte> fileData(fileInfo.EndOfFile.LowPart, 0);

    if (!ReadFile(
        file.get(),
        fileData.data(),
        fileData.size(),
        nullptr,
        nullptr
    ))
    {
        throw winrt::hresult_error(E_UNEXPECTED);
    }

    return fileData;
}

std::future<std::vector<byte>> BasicReaderWriter::ReadDataAsync(
    std::wstring const& filename)
{
    auto file = co_await m_location.GetFileAsync(filename);
    auto buffer = co_await winrt::FileIO::ReadBufferAsync(file);
    std::vector<byte> fileData(buffer.Length(), 0);
    winrt::DataReader::FromBuffer(buffer).ReadBytes(fileData);
    co_return fileData;
}

uint32_t BasicReaderWriter::WriteData(
    std::wstring const& filename,
    std::vector<byte> fileData)
{
    CREATEFILE2_EXTENDED_PARAMETERS extendedParams = {};
    extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
    extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
    extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
    extendedParams.lpSecurityAttributes = nullptr;
    extendedParams.hTemplateFile = nullptr;

    wil::unique_hfile file(
        CreateFile2(
            filename.data(),
            GENERIC_WRITE,
            0,
            CREATE_ALWAYS,
            &extendedParams
        )
    );
    if (file.get() == INVALID_HANDLE_VALUE)
    {
        throw winrt::hresult_error(E_UNEXPECTED);
    }

    DWORD numBytesWritten;
    if (
        !WriteFile(
            file.get(),
            fileData.data(),
            fileData.size(),
            &numBytesWritten,
            nullptr
        ) ||
        numBytesWritten != fileData.size()
        )
    {
        throw winrt::hresult_error(E_UNEXPECTED);
    }

    return numBytesWritten;
}

winrt::IAsyncAction BasicReaderWriter::WriteDataAsync(
    std::wstring const& filename,
    std::vector<byte> fileData)
{
    auto file = co_await m_location.CreateFileAsync(filename, winrt::CreationCollisionOption::ReplaceExisting);
    co_await winrt::FileIO::WriteBytesAsync(file, fileData);
}