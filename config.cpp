#include "config.h"

std::map<std::string, std::string> config; 

bool loadConfig(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }

    file.close();
    return true;
}

std::string getConfigValue(const std::string& key, const std::string& defaultValue) {
    return config.count(key) ? config[key] : defaultValue;
}

int getConfigValue(const std::string& key, int defaultValue) {
    return config.count(key) ? std::stoi(config[key]) : defaultValue;
}