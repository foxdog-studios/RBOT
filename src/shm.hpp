#pragma once
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

namespace fds
{
    constexpr auto sharedDataName = "frame";

    struct SharedData final
    {
        constexpr static auto maxWidth = 1920;
        constexpr static auto maxHeight = 1080;
        constexpr static auto maxChannels = 3;
        boost::interprocess::interprocess_mutex mutex;
        boost::interprocess::interprocess_condition condRead;
        bool requestRead;
        int width;
        int height;
        int channels;
        uint8_t frame[maxWidth * maxHeight * maxChannels];
    };
} // namespace fds
