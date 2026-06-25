#include "sysinfo.hpp"
#include "render.hpp"
#include "export.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <cctype>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    bool noASCII = false;
    bool minimal = false;
    bool noFrame = false;
    std::string outputFmt = "";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--no-ascii") {
            noASCII = true;
        } else if (arg == "--minimal") {
            minimal = true;
        } else if (arg == "--noframe") {
            noFrame = true;
        } else if (arg.rfind("--output=", 0) == 0) {
            outputFmt = arg.substr(9);
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [--no-ascii] [--minimal] [--noframe] [--output=json|xml|txt]\n";
            return 0;
        }
    }

    std::string hostname;
    char hostBuffer[256];
    if (gethostname(hostBuffer, sizeof(hostBuffer)) == 0) {
        hostname = hostBuffer;
    } else {
        hostname = "unknown";
    }

    std::string osName = getOSName();
    std::string kernel = runCommand("uname -r");
    std::string uptimeVal = getUptime();
    std::string shellVal = "";
    const char* shellEnv = std::getenv("SHELL");
    if (shellEnv) {
        shellVal = shellEnv;
    } else {
        shellVal = "sh";
    }
    std::string cpuVal = getCPU();
    std::string memRaw = getMemory();
    std::string diskRaw = getDisk();

    std::vector<std::string> pluginKeys;
    std::vector<std::string> pluginVals;

    // Scan ./plugins directory
    std::vector<fs::path> paths;
    if (fs::exists("./plugins") && fs::is_directory("./plugins")) {
        for (const auto& entry : fs::directory_iterator("./plugins")) {
            if (entry.is_regular_file()) {
                paths.push_back(entry.path());
            }
        }
    }
    std::sort(paths.begin(), paths.end());

    for (const auto& p : paths) {
        std::string pathStr = p.string();
        if (access(pathStr.c_str(), X_OK) == 0) {
            std::string out = runCommandWithTimeout(pathStr, 2);
            if (!out.empty()) {
                size_t nl = out.find('\n');
                std::string pluginOut = out;
                if (nl != std::string::npos) {
                    pluginOut = out.substr(0, nl);
                }
                while (!pluginOut.empty() && std::isspace(static_cast<unsigned char>(pluginOut.back()))) {
                    pluginOut.pop_back();
                }
                while (!pluginOut.empty() && std::isspace(static_cast<unsigned char>(pluginOut.front()))) {
                    pluginOut.erase(pluginOut.begin());
                }

                if (!pluginOut.empty()) {
                    size_t colon = pluginOut.find(':');
                    if (colon != std::string::npos) {
                        std::string k = pluginOut.substr(0, colon);
                        std::string v = pluginOut.substr(colon + 1);
                        while (!v.empty() && std::isspace(static_cast<unsigned char>(v.front()))) {
                            v.erase(v.begin());
                        }
                        pluginKeys.push_back(k);
                        pluginVals.push_back(v);
                    } else {
                        std::string name = p.filename().string();
                        size_t dot = name.find('.');
                        if (dot != std::string::npos) {
                            name = name.substr(0, dot);
                        }
                        if (!name.empty()) {
                            name[0] = std::toupper(static_cast<unsigned char>(name[0]));
                        }
                        pluginKeys.push_back(name);
                        pluginVals.push_back(pluginOut);
                    }
                }
            }
        }
    }

    // Intercept output format flag early
    if (!outputFmt.empty()) {
        if (outputFmt == "json") {
            printJSON(hostname, osName, kernel, uptimeVal, shellVal, cpuVal, memRaw, diskRaw, pluginKeys, pluginVals);
            return 0;
        } else if (outputFmt == "xml") {
            printXML(hostname, osName, kernel, uptimeVal, shellVal, cpuVal, memRaw, diskRaw, pluginKeys, pluginVals);
            return 0;
        } else if (outputFmt == "txt") {
            printTXT(hostname, osName, kernel, uptimeVal, shellVal, cpuVal, memRaw, diskRaw, pluginKeys, pluginVals);
            return 0;
        } else {
            std::cerr << "Unknown output format: " << outputFmt << "\n";
            return 1;
        }
    }

    // Memory & Progress Bar
    std::string memVal = memRaw;
    size_t memPctPos = memRaw.find('%');
    if (memPctPos != std::string::npos) {
        std::string pctPart = memRaw.substr(0, memPctPos);
        while (!pctPart.empty() && std::isspace(static_cast<unsigned char>(pctPart.front()))) pctPart.erase(pctPart.begin());
        while (!pctPart.empty() && std::isspace(static_cast<unsigned char>(pctPart.back()))) pctPart.pop_back();
        try {
            int pct = std::stoi(pctPart);
            memVal = getBar(pct) + " " + memRaw;
        } catch (...) {}
    }

    // Disk & Progress Bar
    std::string diskVal = diskRaw;
    size_t diskPctPos = diskRaw.find('%');
    if (diskPctPos != std::string::npos) {
        size_t start = diskPctPos;
        while (start > 0 && std::isdigit(static_cast<unsigned char>(diskRaw[start - 1]))) {
            start--;
        }
        if (start < diskPctPos) {
            std::string pctStr = diskRaw.substr(start, diskPctPos - start);
            try {
                int pct = std::stoi(pctStr);
                diskVal = getBar(pct) + " " + diskRaw;
            } catch (...) {}
        }
    }

    // Colors
    std::string restore = "\033[0m";
    std::string lblue = "\033[01;34m";
    std::string lyellow = "\033[01;33m";
    std::string lcyan = "\033[01;36m";
    std::string white = "\033[01;37m";
    std::string borderCol = lblue;

    // Setup Logo
    std::vector<std::string> logo;
    std::string distroID = getDistroID();
    if (!noASCII) {
        std::string homeDir = "";
        const char* home = std::getenv("HOME");
        if (home) homeDir = home;

        std::vector<std::string> searchPaths;
        searchPaths.push_back("./ascii/" + distroID + ".txt");
        if (!homeDir.empty()) {
            searchPaths.push_back(homeDir + "/.local/share/tinyfetch/ascii/" + distroID + ".txt");
        }
        searchPaths.push_back("/usr/local/share/tinyfetch/ascii/" + distroID + ".txt");
        searchPaths.push_back("/usr/share/tinyfetch/ascii/" + distroID + ".txt");

        std::string asciiPath = "";
        for (const auto& path : searchPaths) {
            if (fs::exists(path)) {
                asciiPath = path;
                break;
            }
        }

        if (asciiPath.empty()) {
            std::string fallback = "linux";
            if (distroID == "darwin") {
                fallback = "darwin";
            }
            std::vector<std::string> fallbackPaths;
            fallbackPaths.push_back("./ascii/" + fallback + ".txt");
            if (!homeDir.empty()) {
                fallbackPaths.push_back(homeDir + "/.local/share/tinyfetch/ascii/" + fallback + ".txt");
            }
            fallbackPaths.push_back("/usr/local/share/tinyfetch/ascii/" + fallback + ".txt");
            fallbackPaths.push_back("/usr/share/tinyfetch/ascii/" + fallback + ".txt");

            for (const auto& path : fallbackPaths) {
                if (fs::exists(path)) {
                    asciiPath = path;
                    break;
                }
            }
        }

        if (!asciiPath.empty()) {
            std::ifstream file(asciiPath);
            if (file.is_open()) {
                std::string line;
                while (std::getline(file, line)) {
                    logo.push_back(line);
                }
            }
        }

        if (logo.empty()) {
            if (distroID == "darwin") {
                logo = {
                    lcyan + "      .---." + restore,
                    lcyan + "     /     \\" + restore,
                    lcyan + "     \\__   /" + restore,
                    lcyan + "    /   `-' \\" + restore,
                    lcyan + "   |         |" + restore,
                    lcyan + "    \\       /" + restore,
                    lcyan + "     `-...-'" + restore
                };
            } else {
                logo = {
                    lyellow + "     .---." + restore,
                    lyellow + "    /     \\" + restore,
                    lblue + "    \\ " + restore + white + "o o" + restore + lblue + " /" + restore,
                    lyellow + "    /  \\-/ \\" + restore,
                    lyellow + "   / /     \\ \\" + restore,
                    lyellow + "  ( (_     _ ) )" + restore,
                    lyellow + "   `(_`---'_)''" + restore
                };
            }
        }
    }

    std::vector<std::string> info = {
        lblue + "Host:" + restore + "   " + hostname,
        lblue + "OS:" + restore + "     " + osName,
        lblue + "Kernel:" + restore + " " + kernel,
        lblue + "Uptime:" + restore + " " + uptimeVal,
        lblue + "Shell:" + restore + "  " + shellVal,
        lblue + "CPU:" + restore + "    " + cpuVal,
        lblue + "Memory:" + restore + " " + memVal,
        lblue + "Disk:" + restore + "   " + diskVal
    };

    for (size_t i = 0; i < pluginKeys.size(); ++i) {
        info.push_back(lblue + pluginKeys[i] + ":" + restore + " " + pluginVals[i]);
    }

    // Scan ./plugins/extended directory
    std::vector<std::string> extInfo;
    bool hasExt = false;
    if (!minimal) {
        std::vector<fs::path> extPaths;
        if (fs::exists("./plugins/extended") && fs::is_directory("./plugins/extended")) {
            for (const auto& entry : fs::directory_iterator("./plugins/extended")) {
                if (entry.is_regular_file()) {
                    extPaths.push_back(entry.path());
                }
            }
        }
        std::sort(extPaths.begin(), extPaths.end());

        for (const auto& p : extPaths) {
            std::string pathStr = p.string();
            if (access(pathStr.c_str(), X_OK) == 0) {
                std::string out = runCommandWithTimeout(pathStr, 2);
                if (!out.empty()) {
                    std::stringstream ss(out);
                    std::string line;
                    std::vector<std::string> lines;
                    while (std::getline(ss, line)) {
                        lines.push_back(line);
                    }
                    if (!lines.empty() && lines.back().empty()) {
                        lines.pop_back();
                    }
                    if (!lines.empty()) {
                        for (const auto& l : lines) {
                            extInfo.push_back(l);
                        }
                        extInfo.push_back("---");
                        hasExt = true;
                    }
                }
            }
        }
    }
    if (!extInfo.empty() && extInfo.back() == "---") {
        extInfo.pop_back();
    }

    int leftW = 0;
    if (!noASCII) {
        for (const auto& line : logo) {
            int rawLen = visualLength(line);
            if (rawLen > leftW) {
                leftW = rawLen;
            }
        }
        if (leftW < 16) {
            leftW = 16;
        }
    }

    int rightW = 0;
    for (const auto& line : info) {
        int rawLen = visualLength(line);
        if (rawLen > rightW) {
            rightW = rawLen;
        }
    }

    int extW = 0;
    if (hasExt) {
        for (const auto& line : extInfo) {
            int rawLen = visualLength(line);
            if (rawLen > extW) {
                extW = rawLen;
            }
        }
        if (extW < 24) {
            extW = 24;
        }
    }

    int termW = getTerminalWidth();

    int minLogoW = leftW;
    if (noASCII) {
        minLogoW = 0;
    }

    if (!noASCII && hasExt) {
        if (termW < 65) {
            noASCII = true;
            minLogoW = 0;
        }
    }

    if (hasExt) {
        if (termW < 45) {
            hasExt = false;
            extInfo.clear();
            extW = 0;
        }
    }

    if (!noASCII && !hasExt) {
        if (termW < 41) {
            noASCII = true;
            minLogoW = 0;
        }
    }

    int totalBorders = 9;
    if (noASCII) {
        totalBorders = 5;
    }
    if (noFrame) {
        totalBorders = 6;
    }

    int available = termW - minLogoW - totalBorders;
    if (hasExt) {
        rightW = available * 50 / 100;
        extW = available - rightW;
        if (rightW < 20) {
            rightW = 20;
        }
        if (extW < 20) {
            extW = 20;
        }
    } else {
        rightW = available;
        if (rightW < 20) {
            rightW = 20;
        }
    }

    int maxLines = info.size();
    if (!noASCII && logo.size() > static_cast<size_t>(maxLines)) {
        maxLines = logo.size();
    }
    if (hasExt && extInfo.size() > static_cast<size_t>(maxLines)) {
        maxLines = extInfo.size();
    }

    auto repeatStr = [](const std::string& s, int count) -> std::string {
        std::string res;
        for (int i = 0; i < count; ++i) res += s;
        return res;
    };

    if (noFrame) {
        for (int i = 0; i < maxLines; ++i) {
            std::string logoPrint = "";
            if (!noASCII && i < static_cast<int>(logo.size())) {
                logoPrint = logo[i];
            }
            int lRaw = visualLength(logoPrint);
            int lPadCount = leftW - lRaw;
            std::string lPadding = (lPadCount > 0) ? repeatStr(" ", lPadCount) : "";

            std::string infoPrint = "";
            if (i < static_cast<int>(info.size())) {
                infoPrint = info[i];
            }
            infoPrint = truncateANSI(infoPrint, rightW);
            int rRaw = visualLength(infoPrint);
            int rPadCount = rightW - rRaw;
            std::string rPadding = (rPadCount > 0) ? repeatStr(" ", rPadCount) : "";

            std::string ePrint = "";
            if (hasExt && i < static_cast<int>(extInfo.size())) {
                ePrint = extInfo[i];
            }
            if (ePrint == "---") {
                ePrint = "\033[00;37m" + repeatStr("╌", extW) + restore;
            } else {
                ePrint = truncateANSI(ePrint, extW);
            }

            std::string sb = "";
            if (!noASCII) {
                sb += " " + logoPrint + lPadding + "   ";
            }
            sb += infoPrint + rPadding;
            if (hasExt) {
                sb += "   " + ePrint;
            }
            std::cout << sb << "\n";
        }
    } else {
        if (!hasExt) {
            if (noASCII) {
                std::string topLine = borderCol + "┌" + repeatStr("─", rightW + 2) + "┐" + restore;
                std::string botLine = borderCol + "└" + repeatStr("─", rightW + 2) + "┘" + restore;
                std::cout << topLine << "\n";
                for (int i = 0; i < maxLines; ++i) {
                    std::string rLine = "";
                    if (i < static_cast<int>(info.size())) {
                        rLine = info[i];
                    }
                    rLine = truncateANSI(rLine, rightW);
                    int rRaw = visualLength(rLine);
                    int rPadCount = rightW - rRaw;
                    std::string rPadding = (rPadCount > 0) ? repeatStr(" ", rPadCount) : "";
                    std::printf("%s│%s %s%s %s│\n", borderCol.c_str(), restore.c_str(), rLine.c_str(), rPadding.c_str(), borderCol.c_str());
                }
                std::cout << botLine << "\n";
            } else {
                std::string topLine = borderCol + "┌" + repeatStr("─", leftW + 2) + "┬" + repeatStr("─", rightW + 2) + "┐" + restore;
                std::string botLine = borderCol + "└" + repeatStr("─", leftW + 2) + "┴" + repeatStr("─", rightW + 2) + "┘" + restore;
                std::cout << topLine << "\n";
                for (int i = 0; i < maxLines; ++i) {
                    std::string logoPrint = "";
                    if (i < static_cast<int>(logo.size())) {
                        logoPrint = logo[i];
                    }
                    int lRaw = visualLength(logoPrint);
                    int lPadCount = leftW - lRaw;
                    std::string lPadding = (lPadCount > 0) ? repeatStr(" ", lPadCount) : "";

                    std::string infoPrint = "";
                    if (i < static_cast<int>(info.size())) {
                        infoPrint = info[i];
                    }
                    infoPrint = truncateANSI(infoPrint, rightW);
                    int rRaw = visualLength(infoPrint);
                    int rPadCount = rightW - rRaw;
                    std::string rPadding = (rPadCount > 0) ? repeatStr(" ", rPadCount) : "";

                    std::printf("%s│%s %s%s %s│%s %s%s %s│\n",
                        borderCol.c_str(), restore.c_str(), logoPrint.c_str(), lPadding.c_str(),
                        borderCol.c_str(), restore.c_str(), infoPrint.c_str(), rPadding.c_str(),
                        borderCol.c_str());
                }
                std::cout << botLine << "\n";
            }
        } else {
            if (noASCII) {
                std::string topLine = borderCol + "┌" + repeatStr("─", rightW + 2) + "┬" + repeatStr("─", extW + 2) + "┐" + restore;
                std::string botLine = borderCol + "└" + repeatStr("─", rightW + 2) + "┴" + repeatStr("─", extW + 2) + "┘" + restore;
                std::cout << topLine << "\n";
                for (int i = 0; i < maxLines; ++i) {
                    std::string rLine = "";
                    if (i < static_cast<int>(info.size())) {
                        rLine = info[i];
                    }
                    rLine = truncateANSI(rLine, rightW);
                    int rRaw = visualLength(rLine);
                    int rPadCount = rightW - rRaw;
                    std::string rPadding = (rPadCount > 0) ? repeatStr(" ", rPadCount) : "";

                    std::string eLine = "";
                    if (i < static_cast<int>(extInfo.size())) {
                        eLine = extInfo[i];
                    }
                    if (eLine == "---") {
                        eLine = "\033[00;37m" + repeatStr("╌", extW) + restore;
                    } else {
                        eLine = truncateANSI(eLine, extW);
                    }
                    int eRaw = visualLength(eLine);
                    int ePadCount = extW - eRaw;
                    std::string ePadding = (ePadCount > 0) ? repeatStr(" ", ePadCount) : "";

                    std::printf("%s│%s %s%s %s│%s %s%s %s│\n",
                        borderCol.c_str(), restore.c_str(), rLine.c_str(), rPadding.c_str(),
                        borderCol.c_str(), restore.c_str(), eLine.c_str(), ePadding.c_str(),
                        borderCol.c_str());
                }
                std::cout << botLine << "\n";
            } else {
                std::string topLine = borderCol + "┌" + repeatStr("─", leftW + 2) + "┬" + repeatStr("─", rightW + 2) + "┬" + repeatStr("─", extW + 2) + "┐" + restore;
                std::string botLine = borderCol + "└" + repeatStr("─", leftW + 2) + "┴" + repeatStr("─", rightW + 2) + "┴" + repeatStr("─", extW + 2) + "┘" + restore;
                std::cout << topLine << "\n";
                for (int i = 0; i < maxLines; ++i) {
                    std::string logoPrint = "";
                    if (i < static_cast<int>(logo.size())) {
                        logoPrint = logo[i];
                    }
                    int lRaw = visualLength(logoPrint);
                    int lPadCount = leftW - lRaw;
                    std::string lPadding = (lPadCount > 0) ? repeatStr(" ", lPadCount) : "";

                    std::string infoPrint = "";
                    if (i < static_cast<int>(info.size())) {
                        infoPrint = info[i];
                    }
                    infoPrint = truncateANSI(infoPrint, rightW);
                    int rRaw = visualLength(infoPrint);
                    int rPadCount = rightW - rRaw;
                    std::string rPadding = (rPadCount > 0) ? repeatStr(" ", rPadCount) : "";

                    std::string ePrint = "";
                    if (i < static_cast<int>(extInfo.size())) {
                        ePrint = extInfo[i];
                    }
                    if (ePrint == "---") {
                        ePrint = "\033[00;37m" + repeatStr("╌", extW) + restore;
                    } else {
                        ePrint = truncateANSI(ePrint, extW);
                    }
                    int eRaw = visualLength(ePrint);
                    int ePadCount = extW - eRaw;
                    std::string ePadding = (ePadCount > 0) ? repeatStr(" ", ePadCount) : "";

                    std::printf("%s│%s %s%s %s│%s %s%s %s│%s %s%s %s│\n",
                        borderCol.c_str(), restore.c_str(), logoPrint.c_str(), lPadding.c_str(),
                        borderCol.c_str(), restore.c_str(), infoPrint.c_str(), rPadding.c_str(),
                        borderCol.c_str(), restore.c_str(), ePrint.c_str(), ePadding.c_str(),
                        borderCol.c_str());
                }
                std::cout << botLine << "\n";
            }
        }
    }

    return 0;
}
