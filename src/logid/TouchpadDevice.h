#ifndef LOGID_TOUCHPADDEVICE_H
#define LOGID_TOUCHPADDEVICE_H

#include <mutex>
#include <string>
#include <vector>

struct libevdev;
struct libevdev_uinput;

namespace logid {
    class TouchpadDevice {
    public:
        static constexpr int default_width = 1000;
        static constexpr int default_height = 1000;
        static constexpr int default_slots = 10;

        TouchpadDevice(const char* name,
                       int width = default_width,
                       int height = default_height,
                       int slots = default_slots);

        ~TouchpadDevice();

        void setRange(int width, int height);

        [[nodiscard]] int width() const;
        [[nodiscard]] int height() const;
        [[nodiscard]] int slots() const;

        void beginContacts(int fingers, int start_x, int start_y);
        void moveContacts(int dx, int dy);
        void endContacts();

    private:
        struct Contact {
            int x;
            int y;
            int tracking_id;
        };

        void _createDevice();
        void _destroyDevice();
        void _sendEvent(uint type, uint code, int value);
        void _sendSyn();
        void _setToolButtons(int fingers, bool down);
        int _clampX(int x) const;
        int _clampY(int y) const;
        void _endContactsLocked();

        std::mutex _input_mutex;
        std::string _name;
        libevdev* _device{};
        libevdev_uinput* _ui_device{};
        int _range_x;
        int _range_y;
        int _max_slots;
        bool _active;
        int _active_fingers;
        int _next_tracking_id;
        std::vector<Contact> _contacts;
    };
}

#endif // LOGID_TOUCHPADDEVICE_H
