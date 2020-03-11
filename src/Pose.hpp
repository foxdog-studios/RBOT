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
            set(this->tx, tx);
        }

        auto setTy(int const ty) noexcept -> void
        {
            set(this->ty, ty);
        }

        auto setTz(int const tz) noexcept -> void
        {
            set(this->tz, tz);
        }

        auto setAlpha(int const alpha) noexcept -> void
        {
            set(this->alpha, alpha);
        }

        auto setBeta(int const beta) noexcept -> void
        {
            set(this->beta, beta);
        }

        auto setGamma(int const gamma) noexcept -> void
        {
            set(this->gamma, gamma);
        }

      private:
        static auto set(float& destination, int const value) noexcept -> void
        {
            destination = static_cast<float>(value);
        }
    };
} // namespace fds
