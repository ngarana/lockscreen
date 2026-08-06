// DesktopIndex.hpp - Freedesktop .desktop application index for the launcher.
//
// Scans the XDG application directories once, parses each entry, and answers
// case-insensitive search queries. Parsing is in pure static helpers so it
// unit-tests without a real filesystem. Launching a result is a user-initiated
// one-shot spawn (decision D1) — see spawnDetached().
#pragma once

#include <string>
#include <vector>

namespace qypr {

struct DesktopEntry {
    std::string id;    // desktop-file id (basename without ".desktop")
    std::string name;  // display name
    std::string exec;  // command, field codes (%f/%U/…) stripped
    std::string icon;  // freedesktop icon name ("" if none)
    bool terminal = false;
};

class DesktopIndex {
public:
    // Scan $XDG_DATA_HOME + $XDG_DATA_DIRS "/applications" (with sensible
    // fallbacks). Later duplicates by desktop-file id are ignored (earlier dirs
    // win, per the spec). Idempotent — clears and rebuilds.
    //
    // Call this at the point the user asks for the index (opening the launcher,
    // clicking a notification), not at startup: the scan reads every .desktop
    // file in the XDG data dirs, which on a cold boot is real disk I/O, and it
    // used to sit in front of the bar's first frame for a feature nobody had
    // requested yet. loaded() lets a caller skip a redundant rescan.
    void load();

    // True once the directory scan has run.
    [[nodiscard]] bool loaded() const { return loaded_; }

    [[nodiscard]] const std::vector<DesktopEntry>& entries() const { return entries_; }

    // Entries whose name matches `query` (case-insensitive), prefix matches
    // first, then substring, each alphabetical. Empty query → all, alphabetical.
    [[nodiscard]] std::vector<const DesktopEntry*> search(const std::string& query) const;

    // Best-effort resolve an app to a launchable entry, for click-to-launch from
    // a notification. `key` is the notification's `desktop-entry` hint or its
    // app-name. Matched (case-insensitively) as: exact desktop-file id, then the
    // id's trailing component (org.mozilla.firefox → firefox), then exact display
    // name, then name prefix. Null when nothing matches.
    [[nodiscard]] const DesktopEntry* resolve(const std::string& key) const;

    // --- pure helpers (static; unit-tested without the filesystem) ---
    // Parse one .desktop file body. False (skip) when it is NoDisplay/Hidden,
    // not Type=Application, or has no Name/Exec.
    static bool parseEntry(const std::string& body, DesktopEntry& out);
    // Strip Exec field codes (%f %F %u %U %i %c %k %v %m) and surrounding space.
    static std::string cleanExec(const std::string& exec);

private:
    std::vector<DesktopEntry> entries_;
    bool loaded_ = false;
};

// Fork + setsid + exec `sh -c <cmd>`, fully detached (no zombie, no controlling
// terminal). `terminal` wraps the command in the user's $TERMINAL when set.
// User-initiated only (D1); returns false if the fork failed.
bool spawnDetached(const std::string& cmd, bool terminal);

}  // namespace qypr
