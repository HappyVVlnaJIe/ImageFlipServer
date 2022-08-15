#pragma once

#include <cstddef>
#include <chrono>

namespace ImageFlipServer
{
    static const std::size_t buffer_size = 8192;
    static const std::chrono::seconds deadline_time = std::chrono::seconds(60);
}