// QSTile.hpp - Quick Settings tile base and concrete UI implementations.
#pragma once

#include "core/Types.hpp"
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
    QSSliderTile(const std::string& icon,
                 std::function<double()> getValue,
                 std::function<void(double)> onValueChange)
        : icon_(icon), getValue_(std::move(getValue)), onValueChange_(std::move(onValueChange)) {}

    Type type() const override { return Type::Slider; }
    void draw(Painter& p, int64_t now) override;
    void onClick(double x, double y) override;
    void onDrag(double x, double y) override;

private:
    void updateValueFromCoord(double x);

    std::string icon_;
    std::function<double()> getValue_;
    std::function<void(double)> onValueChange_;

    Rect sliderTrackBounds_;
};

}  // namespace qypr
