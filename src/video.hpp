#pragma once
#include <filesystem>
#include <memory>
#include <optional>

#include <opencv2/core.hpp>

namespace fds
{
    class Video
    {
      public:
        virtual ~Video(){};
        virtual auto readFrameInto(cv::Mat& destination) -> void = 0;
        virtual auto togglePause() -> void = 0;
    };

    auto findDevicePath() noexcept -> std::optional<std::filesystem::path>;

    auto makeCVV4l2Video(std::filesystem::path const& devicePath,
                     int width,
                     int height) -> std::unique_ptr<Video>;

    auto makeCVFileVideo(std::filesystem::path const& devicePath,
                     int width,
                     int height) -> std::unique_ptr<Video>;

    auto makeSHMVideo() -> std::unique_ptr<Video>;
} // namespace fds
