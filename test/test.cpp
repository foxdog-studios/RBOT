#include <cstring>
#include <iostream>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <opencv2/core.hpp>

#include "../src/Arguments.hpp"
#include "../src/shm.hpp"
#include "../src/video.hpp"

auto main(int argc, char** argv) -> int
{
    using namespace boost::interprocess;
    using namespace fds;

    auto const args = Arguments{argc, argv};
    constexpr auto width = 640;
    constexpr auto height = 480;
    constexpr auto channels = 3;
    auto video = fds::makeCVVideo(args.getDevicePath(), width, height);

    // Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove()
        {
            shared_memory_object::remove(fds::sharedDataName);
        }

        ~shm_remove()
        {
            shared_memory_object::remove(fds::sharedDataName);
        }
    } remover;

    auto shm =
        shared_memory_object{create_only, fds::sharedDataName, read_write};
    shm.truncate(sizeof(SharedData));
    auto region = mapped_region{shm, read_write};
    auto const addr = region.get_address();
    auto const data = new (addr) fds::SharedData{};

    {
        scoped_lock<interprocess_mutex> lock{data->mutex};
        data->requestRead = false;
        data->width = width;
        data->height = height;
        data->channels = channels;
    }

    cv::Mat frame;

    for (auto i = 1;; ++i)
    {
        std::cout << '\r' << i << std::flush;
        video->readFrameInto(frame);
        scoped_lock<interprocess_mutex> lock{data->mutex};
        std::memcpy(data->frame, frame.data, width * height * channels);

        if (!data->requestRead)
        {
            data->requestRead = true;
            data->condRead.notify_one();
        }
    }

    return 0;
}
