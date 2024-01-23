#include <algorithm>

#include "utils.hpp"

std::span<char> to_string(int i, std::span<char> buffer) {
    if (buffer.size() <= 1) {
        return {buffer.data(), buffer.data()};
    }
    char* start = buffer.data();
    char* end = buffer.data() + buffer.size() - 1;
    char* ptr = buffer.data();

    bool negative = i < 0;
    if (negative) {
        i = -i;
    }

    if (i == 0) {
        *ptr = '0';
        ptr++;
    } else {
        while (i != 0 && ptr < end) {
            *ptr = ('0' + i % 10);
            i /= 10;
            ptr++;
        }
        if (negative && ptr < end) {
            *ptr = '-';
            ptr++;
        }
        std::reverse(start, ptr);
    }

    *ptr = '\0';
    return std::span<char>(start, ptr + 1);
}