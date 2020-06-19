#pragma once
#include "xx_uv.h"
namespace UV = xx::Uv;

struct Resolver : UV::Resolver {
    using UV::Resolver::Resolver;
    void Finish(int const& code) override;
};
