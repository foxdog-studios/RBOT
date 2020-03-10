#pragma once

namespace fds
{
    class Pose final
    {
      public:
        float tx;
        float ty;
        float tz;
        float alpha;
        float beta;
        float gamma;

        auto setTx(int const tx) noexcept -> void
        {
            this->tx = static_cast<float>(tx);
        }

        auto setTy(int const ty) noexcept -> void
        {
            this->ty = static_cast<float>(ty);
        }

        auto setTz(int const tz) noexcept -> void
        {
            this->tz = static_cast<float>(tz);
        }

        auto setAlpha(int const alpha) noexcept -> void
        {
            this->alpha = static_cast<float>(alpha);
        }

        auto setBeta(int const beta) noexcept -> void
        {
            this->beta = static_cast<float>(beta);
        }

        auto setGamma(int const gamma) noexcept -> void
        {
            this->gamma = static_cast<float>(gamma);
        }
    };
} // namespace fds
