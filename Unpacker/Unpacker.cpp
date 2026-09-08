#include <windows.h>
#include "../Common/archive_path.h"
#include "../Common/compression_api.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

CompressionApi::LzmsDecompressor g_decompressor;

constexpr int kMaxArchiveEntries = 2000000;
constexpr int kMaxArchivePathBytes = (MAX_PATH - 1) * 4;

bool ReadExact(FILE* file, void* output, size_t size) {
    return size == 0 || fread(output, 1, size, file) == size;
}

bool CanRead(FILE* file, long long fileSize, long long size) {
    long long position = _ftelli64(file);
    return position >= 0 && size >= 0 && position <= fileSize && size <= fileSize - position;
}

void SetColor(int colorCode) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorCode);
}

std::wstring SmartToWide(const std::vector<char>& buffer) {
    if (buffer.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, buffer.data(), -1, NULL, 0);
    if (size > 0) {
        std::vector<wchar_t> wbuf(size);
        MultiByteToWideChar(CP_UTF8, 0, buffer.data(), -1, wbuf.data(), size);
        return std::wstring(wbuf.data());
    }
    size = MultiByteToWideChar(CP_ACP, 0, buffer.data(), -1, NULL, 0);
    if (size > 0) {
        std::vector<wchar_t> wbuf(size);
        MultiByteToWideChar(CP_ACP, 0, buffer.data(), -1, wbuf.data(), size);
        return std::wstring(wbuf.data());
    }
    return L"Unknown_Path";
}

bool DecompressLZMS(const std::vector<char>& input, std::vector<char>& output, size_t originalSize) {
    output.resize(originalSize);
    size_t decompressedSize = 0;
    return g_decompressor.Decompress(
        input.data(), input.size(), output.data(), output.size(), &decompressedSize)
        && decompressedSize == originalSize;
}

void DrawProgressBar(int current, int total, const std::wstring& currentFile) {
    const int barWidth = 30;
    float progress = (total > 0) ? (float)current / total : 1.0f;
    int pos = (int)(barWidth * progress);

    std::wcout << L"\r";
    SetColor(11); std::wcout << L"[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::wcout << L"=";
        else if (i == pos) std::wcout << L">";
        else std::wcout << L" ";
    }
    std::wcout << L"] ";
    SetColor(14); std::wcout << (int)(progress * 100.0) << L"% ";
    SetColor(7);  std::wcout << L"(" << current << L"/" << total << L") ";
    std::wstring displayFile = currentFile;
    if (displayFile.length() > 20) displayFile = L"..." + displayFile.substr(displayFile.length() - 17);
    SetColor(8);  std::wcout << std::left << std::setw(20) << displayFile;
    SetColor(7);
}

bool UnpackFile(const fs::path& packagePath) {
    auto startTime = std::chrono::high_resolution_clock::now();

    FILE* fpPack = nullptr;
    if (_wfopen_s(&fpPack, packagePath.c_str(), L"rb") != 0 || !fpPack) {
        SetColor(12);
        std::wcout << L"\n[错误] 无法打开: " << packagePath.wstring() << L"\n";
        return false;
    }

    _fseeki64(fpPack, 0, SEEK_END);
    long long packageSize = _ftelli64(fpPack);
    _fseeki64(fpPack, 0, SEEK_SET);

    int fileCount = 0;
    if (packageSize < static_cast<long long>(sizeof(fileCount))
        || !ReadExact(fpPack, &fileCount, sizeof(fileCount))
        || fileCount <= 0 || fileCount > kMaxArchiveEntries) {
        std::wcout << L"无效的封包格式或文件已损坏。\n";
        fclose(fpPack);
        return false;
    }

    fs::path outDir = packagePath;
    outDir.replace_extension("");
    outDir += L"_Unpacked";
    std::error_code directoryError;
    fs::create_directories(outDir, directoryError);
    if (directoryError) {
        std::wcout << L"[错误] 无法创建输出目录: " << outDir.wstring() << L"\n";
        fclose(fpPack);
        return false;
    }

    std::wcout << L"正在解压: " << packagePath.filename().wstring() << L"\n";
    std::wcout << L"文件总数: " << fileCount << L"\n\n";

    for (int i = 0; i < fileCount; ++i) {
        int pathLen = 0;
        if (!ReadExact(fpPack, &pathLen, sizeof(pathLen))
            || pathLen <= 0 || pathLen > kMaxArchivePathBytes
            || !CanRead(fpPack, packageSize, pathLen)) {
            std::wcout << L"\n[错误] 第 " << (i + 1) << L" 个条目的路径长度无效。\n";
            fclose(fpPack);
            return false;
        }
        std::vector<char> pathBuf(pathLen + 1, 0);
        if (!ReadExact(fpPack, pathBuf.data(), pathLen)) {
            fclose(fpPack);
            return false;
        }
        std::wstring relPath = SmartToWide(pathBuf);
        if (!ArchivePath::IsSafeRelative(relPath.c_str())) {
            std::wcout << L"\n[错误] 封包包含不安全路径: " << relPath << L"\n";
            fclose(fpPack);
            return false;
        }
        fs::path fullPath = outDir / relPath;

        directoryError.clear();
        fs::create_directories(fullPath.parent_path(), directoryError);
        if (directoryError) {
            std::wcout << L"\n[错误] 无法创建目录: " << fullPath.parent_path().wstring() << L"\n";
            fclose(fpPack);
            return false;
        }

        int size1 = 0, size2 = 0;
        if (!ReadExact(fpPack, &size1, sizeof(size1)) || size1 < 0) {
            std::wcout << L"\n[错误] 文件尺寸字段无效: " << relPath << L"\n";
            fclose(fpPack);
            return false;
        }

        long long posBeforeSize2 = _ftelli64(fpPack);
        if (!ReadExact(fpPack, &size2, sizeof(size2))) {
            std::wcout << L"\n[错误] 文件尺寸字段不完整: " << relPath << L"\n";
            fclose(fpPack);
            return false;
        }

        bool isCompressedFormat = true;
        if (size2 > size1 || size2 < 0 || size1 < 0) {
            isCompressedFormat = false;
        }

        std::vector<char> outData;
        if (isCompressedFormat) {
            int originalSize = size1;
            int finalSize = size2;
            if (!CanRead(fpPack, packageSize, finalSize)) {
                std::wcout << L"\n[错误] 文件数据超出封包边界: " << relPath << L"\n";
                fclose(fpPack);
                return false;
            }
            std::vector<char> fileData(finalSize);
            if (finalSize > 0 && !ReadExact(fpPack, fileData.data(), finalSize)) {
                fclose(fpPack);
                return false;
            }

            if (finalSize < originalSize && finalSize > 0) {
                if (!DecompressLZMS(fileData, outData, originalSize)) {
                    std::wcout << L"\n[错误] LZMS 解压失败: " << relPath
                        << L" (Windows 错误 " << GetLastError() << L")\n";
                    fclose(fpPack);
                    return false;
                }
            }
            else if (finalSize < originalSize) {
                std::wcout << L"\n[错误] 压缩尺寸无效: " << relPath << L"\n";
                fclose(fpPack);
                return false;
            }
            else {
                outData = fileData;
            }
        }
        else {
            if (_fseeki64(fpPack, posBeforeSize2, SEEK_SET) != 0
                || !CanRead(fpPack, packageSize, size1)) {
                std::wcout << L"\n[错误] 旧版文件数据超出封包边界: " << relPath << L"\n";
                fclose(fpPack);
                return false;
            }
            int realSize = size1;
            outData.resize(realSize);
            if (realSize > 0 && !ReadExact(fpPack, outData.data(), realSize)) {
                fclose(fpPack);
                return false;
            }
        }

        FILE* fpOut = nullptr;
        if (_wfopen_s(&fpOut, fullPath.c_str(), L"wb") != 0 || !fpOut) {
            std::wcout << L"\n[错误] 无法创建文件: " << fullPath.wstring() << L"\n";
            fclose(fpPack);
            return false;
        }
        bool writeSucceeded = outData.empty()
            || fwrite(outData.data(), 1, outData.size(), fpOut) == outData.size();
        if (fclose(fpOut) != 0) writeSucceeded = false;
        if (!writeSucceeded) {
            DeleteFileW(fullPath.c_str());
            std::wcout << L"\n[错误] 文件写入失败: " << fullPath.wstring() << L"\n";
            fclose(fpPack);
            return false;
        }

        DrawProgressBar(i + 1, fileCount, relPath);
    }

    fclose(fpPack);
    std::wcout << L"\n\n";
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;
    SetColor(10); std::wcout << L"任务完成!\n"; SetColor(7);
    std::wcout << L"耗时: " << std::fixed << std::setprecision(2) << elapsed.count() << L" 秒\n";
    return true;
}

int wmain(int argc, wchar_t* argv[]) {
    std::wcout.imbue(std::locale("", std::locale::all));
    SetConsoleTitleW(L"解包工具 (兼容旧版 & LZMS)");

    if (argc < 2) {
        SetColor(11);
        std::wcout << L"========================================\n";
        std::wcout << L"      VFS 解包工具     \n";
        std::wcout << L"========================================\n\n";
        SetColor(7);
        std::wcout << L"说明: 自动识别新旧两种封包格式。\n";
        std::wcout << L"使用: 将 .chs 文件拖入此程序。\n\n";
        system("pause");
        return 1;
    }

    bool allSucceeded = true;
    for (int i = 1; i < argc; i++) {
        if (!UnpackFile(argv[i])) allSucceeded = false;
    }

    g_decompressor.Close();
    std::wcout << L"\n所有任务已完成。";
    system("pause");
    return allSucceeded ? 0 : 2;
}
