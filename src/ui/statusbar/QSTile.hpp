// QSTile.hpp - Quick Settings tile base and concrete UI implementations.
#pragma once

#include "core/Types.hpp"
#include <cstdint>
#include <string>
#include <functional>

namespace qypr {

class Painter;
class MprisController;
class WifiBackend;

class QSTile {
public:
    virtual ~QSTile() = default;

    // The panel dispatches input via virtuals on the base. Sub-classes
    // specialise by overriding the ones they care about.
    enum class Type { Toggle, Slider, Info, Header, Power, WifiCombo, Volume, Media };
    virtual Type type() const = 0;
    virtual std::string title() const { return ""; }

    virtual void draw(Painter& p, int64_t now) = 0;

    virtual void onClick(double x, double y) {}
    virtual void onDrag(double x, double y) {}
    virtual bool handleKey(uint32_t keysym) { return false; }

    Rect bounds;
    bool hovered = false;
    Animated hoverAnim_{0.0};
};

class QSToggleTile : public QSTile {
public:
    QSToggleTile(const std::string& title, const std::string& icon,
                 std::function<bool()> isActive, std::function<void()> onToggle,
                 std::function<std::string()> subtitle = nullptr,
                 Color accent = {0, 0, 0, 0})
        : title_(title), icon_(icon), isActive_(std::move(isActive)),
          onToggle_(std::move(onToggle)), subtitle_(std::move(subtitle)),
          accent_(accent) {}

    Type type() const override { return Type::Toggle; }
    std::string title() const override { return title_; }
    void draw(Painter& p, int64_t now) override;
    void onClick(double x, double y) override;

private:
    std::string title_;
    std::string icon_;
    std::function<bool()> isActive_;
    std::function<void()> onToggle_;
    std::function<std::string()> subtitle_;
    Color accent_;
};

class QSSliderTile : public QSTile {
public:
    // `icon` is the static fallback. The optional trio turns the icon into a
    // live button: `dynamicIcon` overrides the glyph per frame (e.g. speaker →
    // speaker-muted), `onIconClick` fires when the icon itself is clicked (the
    // track still adjusts the value), and `dimmed` greys the row to show the
    // control is currently inert (muted). Omit them for a plain slider.
    QSSliderTile(const std::string& icon,
                 std::function<double()> getValue,
                 std::function<void(double)> onValueChange,
                 std::function<std::string()> dynamicIcon = nullptr,
                 std::function<void()> onIconClick = nullptr,
                 std::function<bool()> dimmed = nullptr,
                 const std::string& title = "Q27G41ZDF")
        : icon_(icon), getValue_(std::move(getValue)), onValueChange_(std::move(onValueChange)),
          dynamicIcon_(std::move(dynamicIcon)), onIconClick_(std::move(onIconClick)),
          dimmed_(std::move(dimmed)), title_(title) {}

    Type type() const override { return Type::Slider; }
    std::string title() const override { return title_; }
    void draw(Painter& p, int64_t now) override;
    void onClick(double x, double y) override;
    void onDrag(double x, double y) override;
    bool handleKey(uint32_t keysym) override;

private:
    void updateValueFromCoord(double x);
    bool stepValue(bool up);
    std::string currentIcon() const { return dynamicIcon_ ? dynamicIcon_() : icon_; }

    std::string icon_;
    std::function<double()> getValue_;
    std::function<void(double)> onValueChange_;
    std::function<std::string()> dynamicIcon_;
    std::function<void()> onIconClick_;
    std::function<bool()> dimmed_;
    std::string title_;

    Rect sliderTrackBounds_;
    Rect iconBounds_;
};

class QSInfoTile : public QSTile {
public:
    QSInfoTile(const std::string& title, const std::string& icon,
               std::function<double()> getProgress,
               std::function<std::string()> getInfo)
        : title_(title), icon_(icon), getProgress_(std::move(getProgress)),
          getInfo_(std::move(getInfo)) {}

    Type type() const override { return Type::Info; }
    std::string title() const override { return title_; }
    void draw(Painter& p, int64_t now) override;

private:
    std::string title_;
    std::string icon_;
    std::function<double()> getProgress_;
    std::function<std::string()> getInfo_;
};

// ─── Header (avatar + name) ────────────────────────────────────────────────
// A non-interactive row at the top of the panel showing the current user
// (avatar circle, name, optional subtitle like email/hostname). Spans the
// full panel width; onClick is a no-op.
class QSHeaderTile : public QSTile {
public:
    QSHeaderTile(std::string title, std::string subtitle, Color avatarColor)
        : title_(std::move(title)), subtitle_(std::move(subtitle)),
          avatarColor_(avatarColor) {}

    Type type() const override { return Type::Header; }
    std::string title() const override { return title_; }
    void draw(Painter& p, int64_t now) override;

private:
    std::string title_;
    std::string subtitle_;
    Color avatarColor_;
};

// ─── Power button ──────────────────────────────────────────────────────────
// Floating circular button on the top-right of the panel. Click triggers the
// supplied callback (typically the system power menu).
class QSPowerTile : public QSTile {
public:
    explicit QSPowerTile(std::function<void()> onClick) : onClick_(std::move(onClick)) {}

    Type type() const override { return Type::Power; }
    std::string title() const override { return "Power"; }
    void draw(Painter& p, int64_t now) override;
    void onClick(double x, double y) override;

private:
    std::function<void()> onClick_;
};

// ─── Wi-Fi combo tile ────────────────────────────────────────────────────
// A toggle (top) with the SSID name and a thin slider beneath showing signal
// strength. Click anywhere on the tile to switch on/off; the slider is purely
// visual.
class QSWifiComboTile : public QSTile {
public:
    QSWifiComboTile(std::string ssid, int strength, bool enabled, bool connected,
                    Color accent, std::function<void()> onToggle = {})
        : ssid_(std::move(ssid)), strength_(strength), enabled_(enabled),
          connected_(connected), accent_(accent), onToggle_(std::move(onToggle)) {}

    // Legacy 4-arg constructor (assumes connected when ssid is non-empty)
    QSWifiComboTile(std::string ssid, int strength, bool enabled, Color accent,
                    std::function<void()> onToggle = {})
        : ssid_(std::move(ssid)), strength_(strength), enabled_(enabled),
          connected_(!ssid_.empty()), accent_(accent), onToggle_(std::move(onToggle)) {}

    Type type() const override { return Type::WifiCombo; }
    std::string title() const override { return "Wi-Fi"; }
    void draw(Painter& p, int64_t now) override;
    void onClick(double, double) override { if (onToggle_) onToggle_(); }

    void setEnabled(bool e) { enabled_ = e; }
    void setConnected(bool c) { connected_ = c; }
    void setSsid(std::string s) { ssid_ = std::move(s); }
    void setStrength(int s) { strength_ = s; }

private:
    std::string ssid_;
    int strength_ = 0;
    bool enabled_ = false;
    bool connected_ = false;
    Color accent_;
    std::function<void()> onToggle_;
};

// ─── Volume section ────────────────────────────────────────────────────────
// A full-width labelled slider row. The header says "Volume"; the track spans
// the rest of the row.
class QSVolumeTile : public QSTile {
public:
    QSVolumeTile(std::function<double()> getValue, std::function<void(double)> onChange,
                 std::function<std::string()> dynamicIcon,
                 std::function<void()> onIconClick,
                 std::function<bool()> dimmed);

    Type type() const override { return Type::Volume; }
    std::string title() const override { return "Volume"; }
    void draw(Painter& p, int64_t now) override;
    void onClick(double x, double y) override;
    void onDrag(double x, double y) override;
    bool handleKey(uint32_t keysym) override;

private:
    void updateValueFromCoord(double x);
    bool stepValue(bool up);
    std::string currentIcon() const { return dynamicIcon_ ? dynamicIcon_() : icon_; }

    std::string icon_;
    std::function<double()> getValue_;
    std::function<void(double)> onValueChange_;
    std::function<std::string()> dynamicIcon_;
    std::function<void()> onIconClick_;
    std::function<bool()> dimmed_;

    Rect trackBounds_;
    Rect iconBounds_;
};

// ─── Media card ────────────────────────────────────────────────────────────
// A full-width card that summarises the active MPRIS player (title / artist
// / album) and a row of transport controls.
class QSMediaTile : public QSTile {
public:
    explicit QSMediaTile(MprisController* mpris) : mpris_(mpris) {}

    Type type() const override { return Type::Media; }
    std::string title() const override { return "Media"; }
    void draw(Painter& p, int64_t now) override;
    void onClick(double x, double y) override;

private:
    MprisController* mpris_ = nullptr;
    Rect prevBounds_{0,0,0,0};
    Rect playBounds_{0,0,0,0};
    Rect nextBounds_{0,0,0,0};
};

}  // namespace qypr
