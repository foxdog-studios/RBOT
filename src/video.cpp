#include "video.hpp"

#include <exception>
#include <filesystem>
#include <memory>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

namespace
{
    class CVVideo : public fds::Video
    {
      public:
        CVVideo(std::filesystem::path const& devicePath,
                int const width,
                int const height)
        {
            if (!this->video.open(devicePath, cv::CAP_V4L2))
            {
                throw std::runtime_error{"Unable to open video device\n"};
            }

            if (!this->video.set(cv::CAP_PROP_FRAME_WIDTH, width))
            {
                throw std::runtime_error{"Unable to set capture frame width\n"};
            }

            if (!this->video.set(cv::CAP_PROP_FRAME_HEIGHT, height))
            {
                throw std::runtime_error{
                    "Unable to set capture frame height\n"};
            }
        }

        ~CVVideo() override
        {
            this->video.release();
        }

        auto tryReadFrameInto(cv::Mat& destination) -> void override
        {
            this->video >> destination;
        }

      private:
        cv::VideoCapture video = cv::VideoCapture{};
    };

    class SHMVideo : public fds::Video
    {
      public:
        auto tryReadFrameInto(cv::Mat& destination) -> void
        {
        }
    };
} // namespace

namespace fds
{
    auto makeCVVideo(std::filesystem::path const& devicePath,
                     int const width,
                     int const height) -> std::unique_ptr<Video>
    {
        return std::make_unique<CVVideo>(devicePath, width, height);
    }

    auto makeSHMVideo() -> std::unique_ptr<Video>
    {
        return std::make_unique<SHMVideo>();
    }
} // namespace fds
