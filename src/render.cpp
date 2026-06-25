#include "render.hpp"
#include <iostream>
#include <vector>

char32_t decodeNextUTF8(const std::string& s, size_t& i) {
    if (i >= s.size()) return 0;
    unsigned char c = s[i];
    if (c < 0x80) {
        i += 1;
        return c;
    }
    if ((c & 0xE0) == 0xC0) {
        if (i + 1 < s.size()) {
            char32_t r = ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[i+1]) & 0x3F);
            i += 2;
            return r;
        }
    } else if ((c & 0xF0) == 0xE0) {
        if (i + 2 < s.size()) {
            char32_t r = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(s[i+1]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i+2]) & 0x3F);
            i += 3;
            return r;
        }
    } else if ((c & 0xF8) == 0xF0) {
        if (i + 3 < s.size()) {
            char32_t r = ((c & 0x07) << 18) | ((static_cast<unsigned char>(s[i+1]) & 0x3F) << 12) | ((static_cast<unsigned char>(s[i+2]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i+3]) & 0x3F);
            i += 4;
            return r;
        }
    }
    // invalid byte, skip
    i += 1;
    return c;
}

int runeWidth(char32_t r) {
    if (r == 0x200d || r == 0x200c || (r >= 0xfe00 && r <= 0xfe0f)) {
        return 0;
    }
    if (r >= 0x0300 && r <= 0x036F) {
        return 0;
    }
    if (r >= 0x1F000 && r <= 0x1FAFF) {
        return 2;
    }
    if (r >= 0x2600 && r <= 0x27BF) {
        return 2;
    }
    if ((r >= 0x2E80 && r <= 0x2FDF) ||
        (r >= 0x3000 && r <= 0x9FFF) ||
        (r >= 0xF900 && r <= 0xFAFF) ||
        (r >= 0xFF01 && r <= 0xFF60) ||
        (r >= 0xFFE0 && r <= 0xFFE6)) {
        return 2;
    }
    return 1;
}

std::string stripANSI(const std::string& s) {
    std::string result;
    bool inEscape = false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\033') {
            inEscape = true;
            continue;
        }
        if (inEscape) {
            if (s[i] == 'm') {
                inEscape = false;
            }
            continue;
        }
        result.push_back(s[i]);
    }
    return result;
}

int visualLength(const std::string& s) {
    std::string raw = stripANSI(s);
    int length = 0;
    size_t i = 0;
    while (i < raw.size()) {
        char32_t r = decodeNextUTF8(raw, i);
        length += runeWidth(r);
    }
    return length;
}

std::string truncateANSI(const std::string& s, int limit) {
    std::string raw = stripANSI(s);
    if (visualLength(raw) <= limit) {
        return s;
    }

    std::string builder;
    int visualLen = 0;
    bool inEscape = false;
    std::string restoreCode = "\033[0m";
    int targetLen = limit - 1;
    if (targetLen < 0) {
        targetLen = 0;
    }

    for (size_t i = 0; i < s.size(); ) {
        if (s[i] == '\033') {
            inEscape = true;
            builder.push_back(s[i]);
            i++;
            continue;
        }
        if (inEscape) {
            builder.push_back(s[i]);
            if (s[i] == 'm') {
                inEscape = false;
            }
            i++;
            continue;
        }

        if (visualLen < targetLen) {
            size_t start = i;
            char32_t r = decodeNextUTF8(s, i);
            int w = runeWidth(r);
            if (visualLen + w <= targetLen) {
                builder.append(s.substr(start, i - start));
                visualLen += w;
            } else {
                visualLen = targetLen;
            }
        } else {
            i++;
        }
    }
    builder.append("…");
    builder.append(restoreCode);
    return builder;
}

std::string getBar(int pct) {
    if (pct < 0) {
        pct = 0;
    }
    int filled = pct / 10;
    if (filled > 10) {
        filled = 10;
    }
    int empty = 10 - filled;
    std::string color = "\033[01;32m"; // Green
    if (pct > 80) {
        color = "\033[01;31m"; // Red
    } else if (pct > 50) {
        color = "\033[01;33m"; // Yellow
    }
    std::string restore = "\033[0m";
    std::string gray = "\033[00;37m";

    std::string sb;
    sb.append(color);
    for (int i = 0; i < filled; i++) {
        sb.append("█");
    }
    sb.append(restore + gray);
    for (int i = 0; i < empty; i++) {
        sb.append("░");
    }
    sb.append(restore);
    return sb;
}
