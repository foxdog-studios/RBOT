#pragma once
#include <filesystem>
#include <optional>
#include <vector>

namespace fds
{
    class Arguments final
    {
      public:
        Arguments(int argc, char** argv) noexcept;
        auto getDevicePath() const -> std::filesystem::path;
        auto getGenerateObjectTemplates() const noexcept -> bool;
        auto getObjectPath() const noexcept -> std::filesystem::path;
        auto getQualityThreshold() const noexcept -> float;
        auto getTemplateDistances() const noexcept -> const std::vector<float>;
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
        float qualityThreshold;
        std::vector<float> templateDistances;
        VideoSource videoSource;
        float zDistance;
    };
} // namespace fds
