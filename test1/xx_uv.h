#pragma once

#include <mutex>
#include <deque>
#include "uv.h"
#include "xx_data.h"
#include "ikcp.h"

namespace xx::Uv {
    struct Context;
    struct Item : std::enable_shared_from_this<Item> {
        std::shared_ptr<Context> uc;
        explicit Item(std::shared_ptr<Context> const& ec);
        virtual ~Item() {}
        void Hold();
        void DelayUnhold();
    };
}
