#pragma once
#include <filesystem>
#include <memory>

#include <opencv2/core.hpp>

namespace fds
{
    class Video
    {
      public:
        virtual ~Video(){};
        virtual auto tryReadFrameInto(cv::Mat& destination) -> void = 0;
    };

    auto makeCVVideo(std::filesystem::path const& devicePath,
                     int width,
                     int height) -> std::unique_ptr<Video>;

    auto makeSHMVideo() -> std::unique_ptr<Video>;
} // namespace fds
