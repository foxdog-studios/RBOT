#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <variant>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <cxxopts.hpp>

#include <opencv2/core.hpp>

#include "../src/shm.hpp"
#include "../src/video.hpp"

auto main(int argc, char** argv) -> int
{
    using namespace boost::interprocess;
    using namespace fds;

    auto options = cxxopts::Options{"shmvideo"};
    auto devicePathValue = cxxopts::value<std::filesystem::path>();

    if (auto const devicePath = fds::findDevicePath(); devicePath)
    {
        devicePathValue->default_value(devicePath.value());
    }

    auto addOption = options.add_options();
    addOption("h,help", "print this message");
    addOption("d,device", "video device path", devicePathValue);

    auto result = [&]() -> std::variant<cxxopts::ParseResult, std::string> {
        try
        {
            return {options.parse(argc, argv)};
        }
        catch (cxxopts::invalid_option_format_error const& error)
        {
            return {error.what()};
        }
        catch (cxxopts::option_not_exists_exception const& error)
        {
            return {error.what()};
        }
        catch (cxxopts::option_requires_argument_exception const& error)
        {
            return {error.what()};
        }
    }();

    if (std::holds_alternative<std::string>(result))
    {
        auto const& error = std::get<std::string>(result);
        std::cout << error << '\n' << std::flush;
        return 1;
    }

    auto const& args = std::get<cxxopts::ParseResult>(result);

    if (args.count("help"))
    {
        std::cout << options.help() << std::flush;
        return 0;
    }

    if (!devicePathValue->has_default() && args.count("device") == 0)
    {
        std::cerr << "Cannot find a C920 camera, please use -d/--device\n"
                  << std::flush;
        return 1;
    }

    constexpr auto width = 640;
    constexpr auto height = 480;
    constexpr auto channels = 3;

    auto const devicePath = args["device"].as<std::filesystem::path>();
    std::cout << "Opening " << devicePath << '\n' << std::flush;
    auto video = fds::makeCVV4l2Video(devicePath, width, height);

    struct SHMRemove
    {
        SHMRemove()
        {
            shared_memory_object::remove(fds::sharedDataName);
        }

        ~SHMRemove()
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
