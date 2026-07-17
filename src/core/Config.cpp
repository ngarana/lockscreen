#include "core/Config.hpp"

#include <cstdlib>
#include <fstream>

namespace qypr {

namespace {

std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    const size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    const size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

std::string lower(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c += 32;
    }
    return s;
}

bool isComment(const std::string& line) {
    return line.empty() || line[0] == '#' || line.rfind("//", 0) == 0;
}

}  // namespace

std::string Config::configDir() {
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME"); xdg && *xdg) {
        return std::string(xdg) + "/qypr";
    }
    if (const char* home = std::getenv("HOME"); home && *home) {
        return std::string(home) + "/.config/qypr";
    }
    return ".";  // last resort: cwd, so a missing HOME cannot crash startup
}

std::string Config::defaultPath() { return configDir() + "/bar.conf"; }

std::string Config::makeKey(const std::string& section, const std::string& key) {
    return section + "." + key;
}

bool Config::load(const std::string& path) {
    path_ = path.empty() ? defaultPath() : path;
    values_.clear();
    loaded_ = false;

    std::ifstream f(path_);
    if (!f.is_open()) return false;  // not an error: compiled defaults apply

    std::string line, section;
    while (std::getline(f, line)) {
        line = trim(line);
        if (isComment(line)) continue;

        if (line.front() == '[') {
            const size_t close = line.find(']');
            if (close != std::string::npos) section = trim(line.substr(1, close - 1));
            continue;
        }

        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;  // ignore junk rather than fail
        const std::string key = trim(line.substr(0, eq));
        if (key.empty()) continue;
        values_[makeKey(section, key)] = trim(line.substr(eq + 1));
    }

    loaded_ = true;
    return true;
}

bool Config::has(const std::string& section, const std::string& key) const {
    return values_.count(makeKey(section, key)) > 0;
}

std::string Config::getString(const std::string& section, const std::string& key,
                              const std::string& def) const {
    auto it = values_.find(makeKey(section, key));
    return it == values_.end() ? def : it->second;
}

int Config::getInt(const std::string& section, const std::string& key, int def) const {
    auto it = values_.find(makeKey(section, key));
    if (it == values_.end()) return def;
    try {
        return std::stoi(it->second);
    } catch (...) {
        return def;  // malformed value: keep the default rather than abort
    }
}

double Config::getDouble(const std::string& section, const std::string& key, double def) const {
    auto it = values_.find(makeKey(section, key));
    if (it == values_.end()) return def;
    try {
        return std::stod(it->second);
    } catch (...) {
        return def;
    }
}

bool Config::getBool(const std::string& section, const std::string& key, bool def) const {
    auto it = values_.find(makeKey(section, key));
    if (it == values_.end()) return def;
    const std::string v = lower(it->second);
    if (v == "true" || v == "yes" || v == "on" || v == "1") return true;
    if (v == "false" || v == "no" || v == "off" || v == "0") return false;
    return def;
}

std::vector<std::string> Config::getList(const std::string& section, const std::string& key,
                                         const std::vector<std::string>& def) const {
    auto it = values_.find(makeKey(section, key));
    if (it == values_.end()) return def;

    // An explicitly empty value means "this zone is empty" — not "use defaults".
    std::vector<std::string> out;
    const std::string& v = it->second;
    size_t start = 0;
    while (start <= v.size()) {
        const size_t comma = v.find(',', start);
        const std::string item =
            trim(v.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
        if (!item.empty()) out.push_back(item);
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return out;
}

}  // namespace qypr
