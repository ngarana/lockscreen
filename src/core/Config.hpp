// Config.hpp - Tiny INI-style configuration reader.
//
// qypr's one user-facing config surface. Deliberately hand-rolled (~150 LOC, no
// new dependency — see STATUS_BAR.md decision D2) and read at startup (D3).
// The [theme] section is live-reloaded via ConfigWatcher (inotify); structural
// keys (modules, position) still require a restart.
//
// Format:
//   # full-line comments (# or //)
//   [section]
//   key = value          ; value keeps inner spaces, ends are trimmed
//   list = a, b, c        ; getList() splits on commas
//
// Only full-line comments are supported: a trailing "#" would mangle values
// like a strftime format. A missing or unreadable file is never an error —
// every accessor falls back to its default, so the compiled-in bar still runs.

#pragma once

#include <map>
#include <string>
#include <vector>

namespace qypr {

class Config {
public:
    // Load `path`, or defaultPath() when empty. Returns whether a file was
    // actually read; the object is usable (all defaults) either way.
    bool load(const std::string& path = "");

    bool loaded() const { return loaded_; }
    const std::string& path() const { return path_; }

    bool has(const std::string& section, const std::string& key) const;

    std::string getString(const std::string& section, const std::string& key,
                          const std::string& def = "") const;
    int getInt(const std::string& section, const std::string& key, int def) const;
    double getDouble(const std::string& section, const std::string& key, double def) const;
    // true/yes/on/1 (case-insensitive) are true; false/no/off/0 are false.
    bool getBool(const std::string& section, const std::string& key, bool def) const;
    // Comma-separated list; entries trimmed, empties dropped. Returns `def` when
    // the key is absent — an explicitly empty value yields an empty list, so a
    // zone can be deliberately emptied.
    std::vector<std::string> getList(const std::string& section, const std::string& key,
                                     const std::vector<std::string>& def = {}) const;

    // $XDG_CONFIG_HOME/qypr, else $HOME/.config/qypr. No trailing slash.
    static std::string configDir();
    // configDir() + "/bar.conf"
    static std::string defaultPath();

private:
    static std::string makeKey(const std::string& section, const std::string& key);

    std::map<std::string, std::string> values_;  // "section.key" -> raw value
    bool loaded_ = false;
    std::string path_;
};

}  // namespace qypr
