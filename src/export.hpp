#pragma once
#include <string>
#include <vector>

void printJSON(const std::string& host, const std::string& osName, const std::string& kernel,
               const std::string& uptime, const std::string& shell, const std::string& cpu,
               const std::string& mem, const std::string& disk,
               const std::vector<std::string>& keys, const std::vector<std::string>& vals);

void printXML(const std::string& host, const std::string& osName, const std::string& kernel,
              const std::string& uptime, const std::string& shell, const std::string& cpu,
              const std::string& mem, const std::string& disk,
              const std::vector<std::string>& keys, const std::vector<std::string>& vals);

void printTXT(const std::string& host, const std::string& osName, const std::string& kernel,
              const std::string& uptime, const std::string& shell, const std::string& cpu,
              const std::string& mem, const std::string& disk,
              const std::vector<std::string>& keys, const std::vector<std::string>& vals);
