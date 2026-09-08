#pragma once

#include <windows.h>
#include <cwchar>

namespace ArchivePath {
namespace Detail {

inline wchar_t ToUpperAscii(wchar_t value) {
    return value >= L'a' && value <= L'z' ? value - (L'a' - L'A') : value;
}

inline bool EqualsAsciiInsensitive(
    const wchar_t* value, size_t length, const wchar_t* expected) {
    size_t index = 0;
    while (index < length && expected[index] != L'\0') {
        if (ToUpperAscii(value[index]) != expected[index]) return false;
        ++index;
    }
    return index == length && expected[index] == L'\0';
}

inline bool IsReservedDeviceName(const wchar_t* segment, size_t length) {
    size_t stemLength = 0;
    while (stemLength < length && segment[stemLength] != L'.') ++stemLength;

    if (EqualsAsciiInsensitive(segment, stemLength, L"CON") ||
        EqualsAsciiInsensitive(segment, stemLength, L"PRN") ||
        EqualsAsciiInsensitive(segment, stemLength, L"AUX") ||
        EqualsAsciiInsensitive(segment, stemLength, L"NUL") ||
        EqualsAsciiInsensitive(segment, stemLength, L"CONIN$") ||
        EqualsAsciiInsensitive(segment, stemLength, L"CONOUT$")) {
        return true;
    }

    if (stemLength == 4) {
        const wchar_t first = ToUpperAscii(segment[0]);
        const wchar_t second = ToUpperAscii(segment[1]);
        const wchar_t third = ToUpperAscii(segment[2]);
        const bool numberedDevice = segment[3] >= L'1' && segment[3] <= L'9';
        return numberedDevice &&
            ((first == L'C' && second == L'O' && third == L'M') ||
             (first == L'L' && second == L'P' && third == L'T'));
    }
    return false;
}

inline bool IsInvalidFileNameCharacter(wchar_t value) {
    return value < L' ' || value == L'<' || value == L'>' ||
        value == L':' || value == L'"' || value == L'|' ||
        value == L'?' || value == L'*';
}

} // namespace Detail

inline bool IsSafeRelative(const wchar_t* path) {
    if (!path || path[0] == L'\0' || path[0] == L'\\' || path[0] == L'/') {
        return false;
    }

    size_t length = 0;
    while (length < MAX_PATH && path[length] != L'\0') ++length;
    if (length == 0 || length >= MAX_PATH) return false;

    size_t segmentStart = 0;
    for (size_t index = 0; index <= length; ++index) {
        const wchar_t value = path[index];
        const bool separator = value == L'\\' || value == L'/';
        if (index < length && !separator) {
            if (Detail::IsInvalidFileNameCharacter(value)) return false;
            continue;
        }

        const size_t segmentLength = index - segmentStart;
        if (segmentLength == 0) return false;
        if ((segmentLength == 1 && path[segmentStart] == L'.') ||
            (segmentLength == 2 && path[segmentStart] == L'.' &&
                path[segmentStart + 1] == L'.')) {
            return false;
        }

        const wchar_t finalCharacter = path[index - 1];
        if (finalCharacter == L'.' || finalCharacter == L' ' ||
            Detail::IsReservedDeviceName(path + segmentStart, segmentLength)) {
            return false;
        }
        segmentStart = index + 1;
    }
    return true;
}

} // namespace ArchivePath

