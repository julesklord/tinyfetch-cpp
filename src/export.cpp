#include "export.hpp"
#include "render.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

static std::string escapeJSON(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\b') result += "\\b";
        else if (c == '\f') result += "\\f";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\t') result += "\\t";
        else if (static_cast<unsigned char>(c) < 0x20) {
            char buf[8];
            std::sprintf(buf, "\\u%04x", static_cast<int>(c));
            result += buf;
        } else {
            result.push_back(c);
        }
    }
    return result;
}

static std::string escapeXML(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '&') result += "&amp;";
        else if (c == '<') result += "&lt;";
        else if (c == '>') result += "&gt;";
        else if (c == '"') result += "&quot;";
        else if (c == '\'') result += "&apos;";
        else result.push_back(c);
    }
    return result;
}

void printJSON(const std::string& host, const std::string& osName, const std::string& kernel,
               const std::string& uptime, const std::string& shell, const std::string& cpu,
               const std::string& mem, const std::string& disk,
               const std::vector<std::string>& keys, const std::vector<std::string>& vals) {
    std::cout << "{\n";
    std::cout << "  \"host\": \"" << escapeJSON(host) << "\",\n";
    std::cout << "  \"os\": \"" << escapeJSON(osName) << "\",\n";
    std::cout << "  \"kernel\": \"" << escapeJSON(kernel) << "\",\n";
    std::cout << "  \"uptime\": \"" << escapeJSON(uptime) << "\",\n";
    std::cout << "  \"shell\": \"" << escapeJSON(shell) << "\",\n";
    std::cout << "  \"cpu\": \"" << escapeJSON(cpu) << "\",\n";
    std::cout << "  \"memory\": \"" << escapeJSON(mem) << "\",\n";
    std::cout << "  \"disk\": \"" << escapeJSON(disk) << "\"";

    if (!keys.empty()) {
        std::cout << ",\n  \"plugins\": {\n";
        for (size_t i = 0; i < keys.size(); ++i) {
            std::string cleanVal = stripANSI(vals[i]);
            std::cout << "    \"" << escapeJSON(keys[i]) << "\": \"" << escapeJSON(cleanVal) << "\"";
            if (i < keys.size() - 1) {
                std::cout << ",\n";
            } else {
                std::cout << "\n";
            }
        }
        std::cout << "  }\n";
    } else {
        std::cout << "\n";
    }
    std::cout << "}\n";
}

void printXML(const std::string& host, const std::string& osName, const std::string& kernel,
              const std::string& uptime, const std::string& shell, const std::string& cpu,
              const std::string& mem, const std::string& disk,
              const std::vector<std::string>& keys, const std::vector<std::string>& vals) {
    std::cout << "<tinyfetch>\n";
    std::cout << "  <host>" << escapeXML(host) << "</host>\n";
    std::cout << "  <os>" << escapeXML(osName) << "</os>\n";
    std::cout << "  <kernel>" << escapeXML(kernel) << "</kernel>\n";
    std::cout << "  <uptime>" << escapeXML(uptime) << "</uptime>\n";
    std::cout << "  <shell>" << escapeXML(shell) << "</shell>\n";
    std::cout << "  <cpu>" << escapeXML(cpu) << "</cpu>\n";
    std::cout << "  <memory>" << escapeXML(mem) << "</memory>\n";
    std::cout << "  <disk>" << escapeXML(disk) << "</disk>\n";
    if (!keys.empty()) {
        std::cout << "  <plugins>\n";
        for (size_t i = 0; i < keys.size(); ++i) {
            std::string tag;
            for (char c : keys[i]) {
                char lower = std::tolower(static_cast<unsigned char>(c));
                if ((lower >= 'a' && lower <= 'z') || (lower >= '0' && lower <= '9')) {
                    tag.push_back(lower);
                } else {
                    tag.push_back('_');
                }
            }
            std::string cleanVal = stripANSI(vals[i]);
            std::cout << "    <" << tag << ">" << escapeXML(cleanVal) << "</" << tag << ">\n";
        }
        std::cout << "  </plugins>\n";
    }
    std::cout << "</tinyfetch>\n";
}

void printTXT(const std::string& host, const std::string& osName, const std::string& kernel,
              const std::string& uptime, const std::string& shell, const std::string& cpu,
              const std::string& mem, const std::string& disk,
              const std::vector<std::string>& keys, const std::vector<std::string>& vals) {
    std::cout << "Host: " << host << "\n";
    std::cout << "OS: " << osName << "\n";
    std::cout << "Kernel: " << kernel << "\n";
    std::cout << "Uptime: " << uptime << "\n";
    std::cout << "Shell: " << shell << "\n";
    std::cout << "CPU: " << cpu << "\n";
    std::cout << "Memory: " << mem << "\n";
    std::cout << "Disk: " << disk << "\n";
    for (size_t i = 0; i < keys.size(); ++i) {
        std::cout << keys[i] << ": " << stripANSI(vals[i]) << "\n";
    }
}
