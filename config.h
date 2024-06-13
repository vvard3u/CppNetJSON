#ifndef CONFIG_H
#define CONFIG_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <csignal>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <filesystem>
#include <iostream>

bool loadConfig(const char* filename);
std::string getConfigValue(const std::string& key, const std::string& defaultValue);
int getConfigValue(const std::string& key, int defaultValue);

extern std::map<std::string, std::string> config;

#endif // CONFIG_H

