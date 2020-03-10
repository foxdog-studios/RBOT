#pragma once
#include <filesystem>
#include <optional>

namespace fds
{
    class Arguments final
    {
      public:
        Arguments(int argc, char** argv) noexcept;
        auto getDevicePath() const -> std::filesystem::path;
        auto getGenerateObjectTemplates() const noexcept -> bool;
        auto getObjectPath() const noexcept -> std::filesystem::path;
        auto getZDistance() const noexcept -> float;
        auto useCVVideo() const noexcept -> bool;
        auto useSHMVideo() const noexcept -> bool;

      private:
        enum class VideoSource
        {
            cv,
            shm,
        };

        std::optional<std::filesystem::path> devicePath;
        bool generateObjectTemplates;
        std::filesystem::path objectPath;
        VideoSource videoSource;
        float zDistance;
    };
} // namespace fds
