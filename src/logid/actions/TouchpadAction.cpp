#include <actions/TouchpadAction.h>
#include <Device.h>
#include <backend/hidpp20/features/ReprogControls.h>
#include <cmath>

using namespace logid::actions;
using namespace logid::backend;

const char* TouchpadAction::interface_name = "Touchpad";

namespace {
    constexpr int default_fingers = 3;
    constexpr double default_scale = 1.0;
    constexpr int max_tool_fingers = 5;
}

TouchpadAction::TouchpadAction(Device* device, config::TouchpadAction& config,
                               [[maybe_unused]] const std::shared_ptr<ipcgull::node>& parent) :
        Action(device, interface_name),
        _fingers(default_fingers),
        _scale(default_scale),
        _width(TouchpadDevice::default_width),
        _height(TouchpadDevice::default_height),
        _invert_x(false),
        _invert_y(false),
        _x_remainder(0.0),
        _y_remainder(0.0),
        _config(config),
        _touchpad(device->virtualTouchpad()) {
}

void TouchpadAction::press() {
    std::unique_lock lock(_config_mutex);
    _pressed = true;
    _applyConfigLocked();
    _x_remainder = 0.0;
    _y_remainder = 0.0;
    if (_touchpad) {
        _touchpad->setRange(_width, _height);
        _touchpad->beginContacts(_fingers, _width / 2, _height / 2);
    }
}

void TouchpadAction::release() {
    std::unique_lock lock(_config_mutex);
    _pressed = false;
    _x_remainder = 0.0;
    _y_remainder = 0.0;
    if (_touchpad)
        _touchpad->endContacts();
}

void TouchpadAction::move(int16_t x, int16_t y) {
    if (!_pressed || !_touchpad)
        return;

    double dx = static_cast<double>(x) * _scale;
    double dy = static_cast<double>(y) * _scale;
    if (_invert_x)
        dx = -dx;
    if (_invert_y)
        dy = -dy;

    const double next_x = _x_remainder + dx;
    const double next_y = _y_remainder + dy;
    const int move_x = static_cast<int>(std::trunc(next_x));
    const int move_y = static_cast<int>(std::trunc(next_y));
    _x_remainder = next_x - move_x;
    _y_remainder = next_y - move_y;

    if (move_x == 0 && move_y == 0)
        return;

    _touchpad->moveContacts(move_x, move_y);
}

uint8_t TouchpadAction::reprogFlags() const {
    return (hidpp20::ReprogControls::TemporaryDiverted |
            hidpp20::ReprogControls::RawXYDiverted);
}

void TouchpadAction::_applyConfigLocked() {
    _fingers = _config.fingers.value_or(default_fingers);
    if (_fingers < 1)
        _fingers = 1;
    if (_fingers > max_tool_fingers)
        _fingers = max_tool_fingers;

    _scale = _config.scale.value_or(default_scale);
    if (_scale <= 0)
        _scale = default_scale;

    _width = _config.width.value_or(TouchpadDevice::default_width);
    if (_width < 1)
        _width = TouchpadDevice::default_width;

    _height = _config.height.value_or(TouchpadDevice::default_height);
    if (_height < 1)
        _height = TouchpadDevice::default_height;

    _invert_x = _config.invert_x.value_or(false);
    _invert_y = _config.invert_y.value_or(false);
}
