#include "Arguments.hpp"

#include <filesystem>
#include <glob.h>
#include <iostream>
#include <string>

#include <cxxopts.hpp>

namespace fds
{
    Arguments::Arguments(int argc, char** argv) noexcept
    {
        auto options = cxxopts::Options{"RBOT"};
        auto device_value = cxxopts::value<std::string>();

        {
            auto result = glob_t{};
            glob("/dev/c920-*", GLOB_ERR, nullptr, &result);

            if (result.gl_pathc != 0)
            {
                device_value->default_value(result.gl_pathv[0]);
            }

            globfree(&result);
        }

        // clang-format off
        options.add_options()
            ("h,help", "print is message")
            ("d,device", "video device path", device_value)
            ("g,gen-object-templates", "generate object templates",
             cxxopts::value<bool>()->default_value("false"))
            ("z,z-distance", "initial z-distance of object",
             cxxopts::value<float>()->default_value("1000"))
            ("object", "object file path", cxxopts::value<std::string>());
        // clang-format on

        options.parse_positional("object");
        options.positional_help("object");

        auto const result = options.parse(argc, argv);

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

        if (!device_value->has_default() && result.count("device") == 0)
        {
            std::cerr << "No video device found, use -d/--device\n"
                      << std::flush;
            ::exit(1);
        }

        this->device_path = result["device"].as<std::string>();
        this->generate_object_templates =
            result["gen-object-templates"].as<bool>();
        this->object_path = result["object"].as<std::string>();
        this->z_distance = result["z-distance"].as<float>();
    }

    auto Arguments::get_device_path() const noexcept -> std::filesystem::path
    {
        return this->device_path;
    }

    auto Arguments::get_generate_object_templates() const noexcept -> bool
    {
        return this->generate_object_templates;
    }

    auto Arguments::get_object_path() const noexcept -> std::filesystem::path
    {
        return this->object_path;
    }

    auto Arguments::get_z_distance() const noexcept -> float
    {
        return this->z_distance;
    }
} // namespace fds
