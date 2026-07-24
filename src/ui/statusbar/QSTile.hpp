// QSTile.hpp - Quick Settings tile base and concrete UI implementations.
#pragma once

#include "core/Types.hpp"
#include <cstdint>
#include <string>
#include <functional>

namespace qypr {

class Painter;

class QSTile {
public:
    virtual ~QSTile() = default;

    enum class Type { Toggle, Slider, Info };
    virtual Type type() const = 0;

    virtual void draw(Painter& p, int64_t now) = 0;

    // Input handlers
    virtual void onClick(double x, double y) {}
    virtual void onDrag(double x, double y) {}
    // Keyboard (only Slider responds today). Returns true when consumed.
    virtual bool handleKey(uint32_t keysym) { return false; }

    Rect bounds;
    bool hovered = false;
    Animated hoverAnim_{0.0};
};

class QSToggleTile : public QSTile {
public:
    QSToggleTile(const std::string& title, const std::string& icon,
                 std::function<bool()> isActive, std::function<void()> onToggle,
                 std::function<std::string()> subtitle = nullptr)
        : title_(title), icon_(icon), isActive_(std::move(isActive)),
          onToggle_(std::move(onToggle)), subtitle_(std::move(subtitle)) {}

    Type type() const override { return Type::Toggle; }
    void draw(Painter& p, int64_t now) override;
    void onClick(double x, double y) override;

private:
    std::string title_;
    std::string icon_;
    std::function<bool()> isActive_;
    std::function<void()> onToggle_;
    std::function<std::string()> subtitle_;
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
                 std::function<bool()> dimmed = nullptr)
        : icon_(icon), getValue_(std::move(getValue)), onValueChange_(std::move(onValueChange)),
          dynamicIcon_(std::move(dynamicIcon)), onIconClick_(std::move(onIconClick)),
          dimmed_(std::move(dimmed)) {}

    Type type() const override { return Type::Slider; }
    void draw(Painter& p, int64_t now) override;
    void onClick(double x, double y) override;
    void onDrag(double x, double y) override;
    bool handleKey(uint32_t keysym) override;

private:
    void updateValueFromCoord(double x);
    // Bump by ±kArrowStep, clamped; respects the dimmed (muted) state by leaving
    // the level where the user left it.
    bool stepValue(bool up);
    std::string currentIcon() const { return dynamicIcon_ ? dynamicIcon_() : icon_; }

    std::string icon_;
    std::function<double()> getValue_;
    std::function<void(double)> onValueChange_;
    std::function<std::string()> dynamicIcon_;
    std::function<void()> onIconClick_;
    std::function<bool()> dimmed_;

    Rect sliderTrackBounds_;
    Rect iconBounds_;  // set in draw(); the icon's click target
};

class QSInfoTile : public QSTile {
public:
    QSInfoTile(const std::string& title, const std::string& icon,
               std::function<double()> getProgress,
               std::function<std::string()> getInfo)
        : title_(title), icon_(icon), getProgress_(std::move(getProgress)),
          getInfo_(std::move(getInfo)) {}

    Type type() const override { return Type::Info; }
    void draw(Painter& p, int64_t now) override;

private:
    std::string title_;
    std::string icon_;
    std::function<double()> getProgress_;
    std::function<std::string()> getInfo_;
};

}  // namespace qypr
