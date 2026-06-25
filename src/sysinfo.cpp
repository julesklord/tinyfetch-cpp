#include "sysinfo.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

std::string runCommand(const std::string& cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return "";
    }
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    // Trim whitespace
    while (!result.empty() && std::isspace(static_cast<unsigned char>(result.back()))) {
        result.pop_back();
    }
    while (!result.empty() && std::isspace(static_cast<unsigned char>(result.front()))) {
        result.erase(result.begin());
    }
    return result;
}

std::string runCommandWithTimeout(const std::string& cmd, int timeoutSec) {
    std::string fullCmd = "timeout " + std::to_string(timeoutSec) + " " + cmd;
    return runCommand(fullCmd);
}

int getTerminalWidth() {
    std::string cols = runCommand("tput cols");
    if (!cols.empty()) {
        try {
            return std::stoi(cols);
        } catch (...) {
            // ignore
        }
    }
    return 80;
}

std::string getOSName() {
    std::ifstream file("/etc/os-release");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind("PRETTY_NAME=", 0) == 0) {
                std::string val = line.substr(12);
                // Strip quotes
                if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                    val = val.substr(1, val.size() - 2);
                }
                return val;
            }
        }
    }
    return "Linux";
}

std::string getDistroID() {
    std::ifstream file("/etc/os-release");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind("ID=", 0) == 0) {
                std::string val = line.substr(3);
                // Strip quotes
                if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                    val = val.substr(1, val.size() - 2);
                }
                return val;
            }
        }
    }
    return "linux";
}

std::string getUptime() {
    std::ifstream file("/proc/uptime");
    if (file.is_open()) {
        double seconds = 0;
        if (file >> seconds) {
            int h = static_cast<int>(seconds) / 3600;
            int m = (static_cast<int>(seconds) % 3600) / 60;
            return std::to_string(h) + "h " + std::to_string(m) + "m";
        }
    }
    return "n/a";
}

std::string getCPU() {
    std::ifstream file("/proc/cpuinfo");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind("model name", 0) == 0) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string cpu = line.substr(colon + 1);
                    // Trim space
                    while (!cpu.empty() && std::isspace(static_cast<unsigned char>(cpu.front()))) {
                        cpu.erase(cpu.begin());
                    }
                    while (!cpu.empty() && std::isspace(static_cast<unsigned char>(cpu.back()))) {
                        cpu.pop_back();
                    }
                    return cpu;
                }
            }
        }
    }
    return "Unknown CPU";
}

std::string getMemory() {
    std::ifstream file("/proc/meminfo");
    if (file.is_open()) {
        std::string line;
        long long total = 0;
        long long avail = 0;
        while (std::getline(file, line)) {
            if (line.rfind("MemTotal:", 0) == 0) {
                std::sscanf(line.c_str(), "MemTotal: %lld kB", &total);
            } else if (line.rfind("MemAvailable:", 0) == 0) {
                std::sscanf(line.c_str(), "MemAvailable: %lld kB", &avail);
            }
        }
        if (total > 0) {
            long long usedPct = (total - avail) * 100 / total;
            return std::to_string(usedPct) + "% (" + std::to_string(total / 1024) + "MB)";
        }
    }
    return "n/a";
}

std::string getDisk() {
    std::string out = runCommand("df -Ph /");
    if (!out.empty()) {
        std::vector<std::string> lines;
        std::string line;
        std::stringstream ss(out);
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }
        if (lines.size() >= 2) {
            std::vector<std::string> fields;
            std::string field;
            std::stringstream ss_fields(lines[1]);
            while (ss_fields >> field) {
                fields.push_back(field);
            }
            if (fields.size() >= 5) {
                return fields[0] + " (" + fields[4] + ")";
            }
        }
    }
    return "n/a";
}
