#include <filesystem>
#include <string>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/persistence.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "Recording.hpp"

namespace fds
{
    Recording::Recording(std::filesystem::path const& recordingDirectory,
                         std::filesystem::path const& modelPath,
                         cv::Matx33f const& K,
                         float const diameter)
        : recordingDirectory{recordingDirectory},
          rgbDirectory{recordingDirectory / "rgb"},
          maskDirectory{recordingDirectory / "mask"},
          poseDirectory{recordingDirectory / "pose"}
    {
        namespace fs = std::filesystem;
        fs::create_directories(recordingDirectory);
        fs::create_directories(rgbDirectory);
        fs::create_directories(maskDirectory);
        fs::create_directories(poseDirectory);
        fs::copy(modelPath,
                 recordingDirectory,
                 fs::copy_options::overwrite_existing);
        auto kFile = cv::FileStorage{recordingDirectory / "camera.yml",
                                     cv::FileStorage::WRITE};
        kFile << "camera" << K;
        auto diameterFile = cv::FileStorage{recordingDirectory / "diameter.yml",
                                            cv::FileStorage::WRITE};
        auto mmToMetres = 0.001;
        diameterFile << "diameter" << diameter * mmToMetres;
    }

    auto Recording::toggleRecording() noexcept -> void
    {
        isRecording = !isRecording;
    }

    auto Recording::getIsRecording() const noexcept -> bool
    {
        return isRecording;
    }

    auto Recording::getFrameCount() const noexcept -> int
    {
        return frameCount;
    }

    auto Recording::update(cv::Mat const& rgb,
                           cv::Mat const& depth,
                           cv::Matx44f const& pose) -> void
    {
        if (!isRecording)
        {
            return;
        }
        auto rgbPath = makePath(rgbDirectory, ".jpg");
        cv::imwrite(rgbPath, rgb);

        auto mask = cv::Mat{depth.rows, depth.cols, depth.type()};
        cv::threshold(depth, mask, 0, 255, cv::THRESH_BINARY);

        auto maskPath = makePath(maskDirectory, ".png");
        cv::imwrite(maskPath, mask);

        auto posePath = makePath(poseDirectory, ".yml");
        auto poseFile = cv::FileStorage{posePath, cv::FileStorage::WRITE};
        poseFile << "pose" << pose;

        frameCount += 1;
    }

    auto Recording::makePath(std::filesystem::path const& directory,
                             std::string const& ext) const noexcept
        -> std::filesystem::path
    {
        return (directory / std::to_string(frameCount)).replace_extension(ext);
    }

} // namespace fds
