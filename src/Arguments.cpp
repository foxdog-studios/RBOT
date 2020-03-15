#include "Arguments.hpp"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include <cxxopts.hpp>

#include "video.hpp"

namespace fds
{
    Arguments::Arguments(int argc, char** argv) noexcept
    {
        auto options = cxxopts::Options{"RBOT"};
        auto device_value = cxxopts::value<std::string>();

        if (auto const devicePath = fds::findDevicePath(); devicePath)
        {
            device_value->default_value(devicePath.value());
        }

        // clang-format off
        options.add_options()
            ("h,help", "print is message")
            ("d,device", "cv video device path", device_value)
            ("g,gen-object-templates", "generate object templates",
             cxxopts::value<bool>()->default_value("false"))
            ("t,template-distances", "template distances in mm, used to track lost objects",
             cxxopts::value<std::vector<float>>()->default_value("500,1000,1200"))
            ("v,video", "video source, either cv or shm",
             cxxopts::value<std::string>()->default_value("cv"))
            ("q,quality-threshold", "quality threshold before object lost",
             cxxopts::value<float>()->default_value("0.55"))
            ("z,z-distance", "initial z-distance of object",
             cxxopts::value<float>()->default_value("1000"))
            ("object", "object file path", cxxopts::value<std::string>());
        // clang-format on

        options.parse_positional("object");
        options.positional_help("object");

        auto const result = [&]() {
            try
            {
                return options.parse(argc, argv);
            }
            catch (cxxopts::option_not_exists_exception& error)
            {
                std::cerr << error.what() << '\n' << std::flush;
                ::exit(0);
            }
        }();

        if (result.count("help") != 0)
        {
            std::cout << options.help() << std::flush;
            ::exit(0);
        }

        if (result.count("object") == 0)
        {
            std::cerr << "An object file is required\n" << std::flush;
            ::exit(1);
        }

        auto const videoSourceStr = result["video"].as<std::string>();

        if (videoSourceStr == "cv")
        {
            this->videoSource = VideoSource::cv;
        }
        else if (videoSourceStr == "shm")
        {
            this->videoSource = VideoSource::shm;
        }
        else
        {
            std::cerr << '"' << videoSourceStr
                      << "\" is not a video source, choice either cv or shm\n"
                      << std::flush;
            ::exit(1);
        }

        if (!device_value->has_default() && result.count("device") == 0)
        {
            std::cerr << "No video device found, use -d/--device\n"
                      << std::flush;
            ::exit(1);
        }

        this->devicePath = result["device"].as<std::string>();
        this->generateObjectTemplates =
            result["gen-object-templates"].as<bool>();
        this->objectPath = result["object"].as<std::string>();
        this->qualityThreshold = result["quality-threshold"].as<float>();
        this->templateDistances = result["template-distances"].as<std::vector<float>>();
        this->zDistance = result["z-distance"].as<float>();
    }

    auto Arguments::getDevicePath() const -> std::filesystem::path
    {

        return this->devicePath.value();
    }

    auto Arguments::getGenerateObjectTemplates() const noexcept -> bool
    {
        return this->generateObjectTemplates;
    }

    auto Arguments::getObjectPath() const noexcept -> std::filesystem::path
    {
        return this->objectPath;
    }

    auto Arguments::useCVVideo() const noexcept -> bool
    {
        return this->videoSource == VideoSource::cv;
    }

    auto Arguments::useSHMVideo() const noexcept -> bool
    {
        return this->videoSource == VideoSource::shm;
    }

    auto Arguments::getQualityThreshold() const noexcept -> float
    {
        return this->qualityThreshold;
    }

    auto Arguments::getTemplateDistances() const noexcept -> const std::vector<float>
    {
        return this->templateDistances;
    }

    auto Arguments::getZDistance() const noexcept -> float
    {
        return this->zDistance;
    }
} // namespace fds
