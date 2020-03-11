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
#include <opencv2/highgui.hpp>

#include "shm.hpp"

namespace
{
    class CVVideo final : public fds::Video
    {
      public:
        CVVideo(std::filesystem::path const& devicePath, int width, int height);
        ~CVVideo() override;
        auto readFrameInto(cv::Mat& destination) -> void override;

      private:
        cv::VideoCapture video = cv::VideoCapture{};
    };

    CVVideo::CVVideo(std::filesystem::path const& devicePath,
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

    CVVideo::~CVVideo()
    {
        this->video.release();
    }

    auto CVVideo::readFrameInto(cv::Mat& destination) -> void
    {
        do
        {
            this->video >> destination;
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
