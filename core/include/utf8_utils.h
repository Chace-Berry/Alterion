#pragma once
#include <string>

struct Utf8Char {
    std::string value;
    int byteCount = 0;
    bool valid = true;
    std::string error;
};

uint32_t decodeUTF8(const std::string &bytes)
{
    const unsigned char *data = reinterpret_cast<const unsigned char *>(bytes.data());
    size_t len = bytes.size();

    if (len == 1)
    {
        return data[0];
    }
    else if (len == 2)
    {
        return ((data[0] & 0x1F) << 6) |
               (data[1] & 0x3F);
    }
    else if (len == 3)
    {
        return ((data[0] & 0x0F) << 12) |
               ((data[1] & 0x3F) << 6) |
               (data[2] & 0x3F);
    }
    else if (len == 4)
    {
        return ((data[0] & 0x07) << 18) |
               ((data[1] & 0x3F) << 12) |
               ((data[2] & 0x3F) << 6) |
               (data[3] & 0x3F);
    }

    return 0xFFFD;
}