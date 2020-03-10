#pragma once
#include <filesystem>

namespace fds
{
    class Arguments final
    {
      public:
        Arguments(int argc, char** argv) noexcept;
        auto get_device_path() const noexcept -> std::filesystem::path;
        auto get_generate_object_templates() const noexcept -> bool;
        auto get_object_path() const noexcept -> std::filesystem::path;
        auto get_z_distance() const noexcept -> float;

      private:
        std::filesystem::path device_path;
        bool generate_object_templates;
        std::filesystem::path object_path;
        float z_distance;
    };
} // namespace fds
