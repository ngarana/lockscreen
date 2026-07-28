// DesktopIndex.cpp - .desktop application index implementation.
#include "system/DesktopIndex.hpp"

#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <unordered_set>

namespace qypr {

namespace {

std::string lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

bool truthy(const std::string& v) {
    return v == "true" || v == "1" || v == "yes";
}

std::string readFile(const std::string& path) {
    std::FILE* f = std::fopen(path.c_str(), "re");
    if (!f) return {};
    std::string out;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) out.append(buf, n);
    std::fclose(f);
    return out;
}

// The application search path: XDG_DATA_HOME then XDG_DATA_DIRS, each +
// "/applications", with the spec's fallbacks.
std::vector<std::string> appDirs() {
    std::vector<std::string> dirs;
    const char* home = std::getenv("HOME");
    const char* dataHome = std::getenv("XDG_DATA_HOME");
    if (dataHome && *dataHome) dirs.push_back(std::string(dataHome) + "/applications");
    else if (home) dirs.push_back(std::string(home) + "/.local/share/applications");

    const char* dataDirs = std::getenv("XDG_DATA_DIRS");
    std::string list = (dataDirs && *dataDirs) ? dataDirs : "/usr/local/share:/usr/share";
    std::stringstream ss(list);
    std::string dir;
    while (std::getline(ss, dir, ':')) {
        if (!dir.empty()) dirs.push_back(dir + "/applications");
    }
    return dirs;
}

}  // namespace

std::string DesktopIndex::cleanExec(const std::string& exec) {
    std::string out;
    out.reserve(exec.size());
    for (size_t i = 0; i < exec.size(); ++i) {
        if (exec[i] == '%' && i + 1 < exec.size()) {
            const char c = exec[i + 1];
            // Freedesktop Exec field codes (incl. deprecated d D n N v m).
            if (std::strchr("fFuUickdDnNvm", c)) {
                ++i;
                continue;
            }
            if (c == '%') {  // "%%" → literal "%"
                out += '%';
                ++i;
                continue;
            }
        }
        out += exec[i];
    }
    // Collapse doubled spaces the removals may have left, and trim.
    std::string tidy;
    bool prevSpace = false;
    for (char c : out) {
        const bool sp = c == ' ' || c == '\t';
        if (sp && prevSpace) continue;
        tidy += c;
        prevSpace = sp;
    }
    size_t a = tidy.find_first_not_of(" \t");
    size_t b = tidy.find_last_not_of(" \t");
    return a == std::string::npos ? "" : tidy.substr(a, b - a + 1);
}

bool DesktopIndex::parseEntry(const std::string& body, DesktopEntry& out) {
    out = {};
    std::string type, name, exec, icon;
    bool noDisplay = false, hidden = false, terminal = false;
    bool inEntry = false;

    std::stringstream ss(body);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        if (line[0] == '[') {
            // Only the main [Desktop Entry] group; stop at the first action group.
            inEntry = line == "[Desktop Entry]";
            continue;
        }
        if (!inEntry) continue;
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = line.substr(0, eq);
        const std::string val = line.substr(eq + 1);
        // Ignore localized keys (Name[de]=…): take the unlocalized value only.
        if (key == "Type") type = val;
        else if (key == "Name") name = val;
        else if (key == "Exec") exec = val;
        else if (key == "Icon") icon = val;
        else if (key == "NoDisplay") noDisplay = truthy(val);
        else if (key == "Hidden") hidden = truthy(val);
        else if (key == "Terminal") terminal = truthy(val);
    }

    if (type != "Application") return false;
    if (noDisplay || hidden) return false;
    if (name.empty() || exec.empty()) return false;

    out.name = name;
    out.exec = cleanExec(exec);
    out.icon = icon;
    out.terminal = terminal;
    return !out.exec.empty();
}

void DesktopIndex::load() {
    entries_.clear();
    std::unordered_set<std::string> seen;  // desktop-file id dedup (earlier dir wins)

    for (const auto& dir : appDirs()) {
        DIR* d = opendir(dir.c_str());
        if (!d) continue;
        struct dirent* de;
        while ((de = readdir(d)) != nullptr) {
            const std::string fname = de->d_name;
            if (fname.size() < 9 || fname.compare(fname.size() - 8, 8, ".desktop") != 0) continue;
            if (!seen.insert(fname).second) continue;  // already provided by an earlier dir
            DesktopEntry e;
            if (parseEntry(readFile(dir + "/" + fname), e)) {
                e.id = fname.substr(0, fname.size() - 8);  // strip ".desktop"
                entries_.push_back(std::move(e));
            }
        }
        closedir(d);
    }

    std::sort(entries_.begin(), entries_.end(),
              [](const DesktopEntry& a, const DesktopEntry& b) { return lower(a.name) < lower(b.name); });
}

std::vector<const DesktopEntry*> DesktopIndex::search(const std::string& query) const {
    std::vector<const DesktopEntry*> prefix, substr;
    const std::string q = lower(query);
    for (const auto& e : entries_) {
        if (q.empty()) {
            prefix.push_back(&e);
            continue;
        }
        const std::string n = lower(e.name);
        const size_t pos = n.find(q);
        if (pos == 0) prefix.push_back(&e);
        else if (pos != std::string::npos) substr.push_back(&e);
    }
    prefix.insert(prefix.end(), substr.begin(), substr.end());  // both already alphabetical
    return prefix;
}

const DesktopEntry* DesktopIndex::resolve(const std::string& key) const {
    if (key.empty()) return nullptr;
    const std::string q = lower(key);
    // The trailing dotted component, so an app-id like "org.telegram.desktop"
    // also matches an entry keyed "telegram", and vice-versa.
    auto tail = [](const std::string& s) {
        const size_t dot = s.rfind('.');
        return dot == std::string::npos ? s : s.substr(dot + 1);
    };
    const std::string qtail = tail(q);

    // 1. exact desktop-file id.
    for (const auto& e : entries_)
        if (lower(e.id) == q) return &e;
    // 2. trailing component of either side matches.
    for (const auto& e : entries_) {
        const std::string id = lower(e.id);
        if (tail(id) == qtail || id == qtail || tail(id) == q) return &e;
    }
    // 3. exact display name, then name prefix.
    for (const auto& e : entries_)
        if (lower(e.name) == q) return &e;
    for (const auto& e : entries_)
        if (lower(e.name).rfind(q, 0) == 0) return &e;
    return nullptr;
}

bool spawnDetached(const std::string& cmd, bool terminal) {
    if (cmd.empty()) return false;
    std::string full = cmd;
    if (terminal) {
        const char* term = std::getenv("TERMINAL");
        full = std::string(term && *term ? term : "xterm") + " -e " + cmd;
    }

    const pid_t pid = fork();
    if (pid < 0) return false;
    if (pid == 0) {
        // Child: new session so it outlives the bar and has no controlling tty.
        setsid();
        // Double-fork so the grandchild is reparented to init (no zombie).
        if (fork() == 0) {
            execl("/bin/sh", "sh", "-c", full.c_str(), nullptr);
            _exit(127);
        }
        _exit(0);
    }
    // Parent: reap the immediate child (the intermediate that exited at once).
    int status = 0;
    waitpid(pid, &status, 0);
    return true;
}

}  // namespace qypr
