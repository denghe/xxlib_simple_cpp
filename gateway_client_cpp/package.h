#pragma once

#include "xx_data.h"

struct Package {
    uint32_t serverId = 0;
    int serial = 0;
    xx::Data data;

    Package(uint32_t const &serverId, int const &serial, xx::Data &&data)
            : serverId(serverId), serial(serial), data(std::move(data)) {}

    Package() = default;

    Package(Package const &) = default;

    Package &operator=(Package const &) = default;

    inline Package(Package &&o) noexcept {
        operator=(std::move(o));
    }

    inline Package &operator=(Package &&o) noexcept {
        std::swap(serverId, o.serverId);
        std::swap(serial, o.serial);
        std::swap(data, o.data);
        return *this;
    }
};
