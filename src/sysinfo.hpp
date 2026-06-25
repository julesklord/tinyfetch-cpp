#pragma once
#include <string>
#include <vector>

std::string runCommand(const std::string& cmd);
std::string runCommandWithTimeout(const std::string& cmd, int timeoutSec);
int getTerminalWidth();
std::string getOSName();
std::string getDistroID();
std::string getUptime();
std::string getCPU();
std::string getMemory();
std::string getDisk();
