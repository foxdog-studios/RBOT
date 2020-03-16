#pragma once

#include <filesystem>

#include <opencv2/core.hpp>

namespace fds
{
    class Recording final
    {
      public:
        explicit Recording(std::filesystem::path recordingDirectory);
        auto toggleRecording() noexcept -> void;
        auto getIsRecording() const noexcept -> bool;
        auto getFrameCount() const noexcept -> int;
        auto update(cv::Mat const& rgb,
                    cv::Mat const& mask,
                    cv::Matx44f const& pose) -> void;

      private:
        bool isRecording = false;
        int frameCount = 0;
        std::filesystem::path const recordingDirectory;
        std::filesystem::path const rgbDirectory;
        std::filesystem::path const maskDirectory;
        std::filesystem::path const poseDirectory;

        auto makePath(std::filesystem::path const &directory, std::string const &ext) const noexcept -> std::filesystem::path;
    };
} // namespace fds
