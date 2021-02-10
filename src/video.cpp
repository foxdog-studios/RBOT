#include "video.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <glob.h>
#include <memory>
#include <optional>
#include <thread>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "shm.hpp"

namespace
{
    class CVV4l2Video final : public fds::Video
    {
      public:
        CVV4l2Video(std::filesystem::path const& devicePath, int width, int height);
        ~CVV4l2Video() override;
        auto readFrameInto(cv::Mat& destination) -> void override;
        auto togglePause() -> void override;

      private:
        cv::VideoCapture video = cv::VideoCapture{};
    };

    CVV4l2Video::CVV4l2Video(std::filesystem::path const& devicePath,
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
            throw std::runtime_error{"Unable to set capture frame height\n"};
        }
    }

    CVV4l2Video::~CVV4l2Video()
    {
        this->video.release();
    }

    auto CVV4l2Video::togglePause() -> void
    {
        // TODO implement me
    }

    auto CVV4l2Video::readFrameInto(cv::Mat& destination) -> void
    {
        do
        {
            this->video >> destination;
        } while (destination.empty());
    }

    class CVFileVideo final : public fds::Video
    {
      public:
        CVFileVideo(std::filesystem::path const& devicePath, int width, int height);
        ~CVFileVideo() override;
        auto readFrameInto(cv::Mat& destination) -> void override;
        auto togglePause() -> void override;

      private:
        cv::VideoCapture video = cv::VideoCapture{};
        cv::Size size;
        cv::Mat lastFrame;
        bool paused = false;
    };

    CVFileVideo::CVFileVideo(std::filesystem::path const& devicePath,
                     int const width,
                     int const height): size(width, height)
    {
        if (!this->video.open(devicePath, cv::CAP_ANY))
        {
            throw std::runtime_error{"Unable to open video device\n"};
        }
    }

    CVFileVideo::~CVFileVideo()
    {
        this->video.release();
    }

    auto CVFileVideo::togglePause() -> void
    {
        this->paused = !this->paused;
    }

    auto CVFileVideo::readFrameInto(cv::Mat& destination) -> void
    {
        if (this->paused && !this->lastFrame.empty()) {
            cv::resize(this->lastFrame, destination, this->size);
            return;
        }
        do
        {
            this->video.read(this->lastFrame);
            if (this->lastFrame.empty()) {
                this->video.set(cv::CAP_PROP_POS_FRAMES, 0);
            } else {
                cv::resize(this->lastFrame, destination, this->size);
            }
        } while (destination.empty());
    }

    class SHMVideo final : public fds::Video
    {
      public:
        SHMVideo()
        {
            using namespace boost::interprocess;

            this->shm = []() {
                for (;;)
                {
                    try
                    {
                        return shared_memory_object{
                            open_only, fds::sharedDataName, read_write};
                    }
                    catch (interprocess_exception&)
                    {
                        std::this_thread::sleep_for(std::chrono::seconds{1});
                    }
                }
            }();

            this->region = mapped_region{shm, read_write};
            this->data = static_cast<fds::SharedData*>(region.get_address());
        }

        auto togglePause() -> void override
        {
            // TODO implement me
        }

        auto readFrameInto(cv::Mat& destination) -> void override
        {
            using namespace boost::interprocess;

            scoped_lock<interprocess_mutex> lock{data->mutex};

            // Wait for the producer to write the next frame.
            if (!data->requestRead)
            {
                data->condRead.wait(lock);
            }

            cv::Mat source{cv::Size(data->width, data->height),
                           CV_8UC3,
                           data->frame,
                           cv::Mat::AUTO_STEP};
            source.copyTo(destination);
            data->requestRead = false;
        }

      private:
        boost::interprocess::shared_memory_object shm;
        boost::interprocess::mapped_region region;
        fds::SharedData* data;
    };
} // namespace

namespace fds
{
    auto makeCVV4l2Video(std::filesystem::path const& devicePath,
                     int const width,
                     int const height) -> std::unique_ptr<Video>
    {
        return std::make_unique<CVV4l2Video>(devicePath, width, height);
    }

    auto makeCVFileVideo(std::filesystem::path const& devicePath,
                     int const width,
                     int const height) -> std::unique_ptr<Video>
    {
        return std::make_unique<CVFileVideo>(devicePath, width, height);
    }

    auto makeSHMVideo() -> std::unique_ptr<Video>
    {
        return std::make_unique<SHMVideo>();
    }

    auto findDevicePath() noexcept -> std::optional<std::filesystem::path>
    {
        struct ManagedGlob
        {
            glob_t data = glob_t{};

            ~ManagedGlob()
            {
                ::globfree(&this->data);
            }
        } result;

        ::glob("/dev/c920-*", GLOB_ERR, nullptr, &result.data);

        if (result.data.gl_pathc == 0)
        {
            return {};
        }

        return {result.data.gl_pathv[0]};
    }

} // namespace fds
