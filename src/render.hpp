#pragma once
#include <string>

int runeWidth(char32_t r);
int visualLength(const std::string& s);
std::string truncateANSI(const std::string& s, int limit);
std::string stripANSI(const std::string& s);
std::string getBar(int pct);
char32_t decodeNextUTF8(const std::string& s, size_t& i);
