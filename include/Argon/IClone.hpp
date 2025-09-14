#ifndef ARGON_ICLONE_HPP
#define ARGON_ICLONE_HPP

#include <memory>

namespace Argon {
    class IClone {
    public:
        virtual ~IClone() = default;

        virtual auto clone() const -> std::unique_ptr<IClone> = 0;

        template <typename T>
        auto clone_as() -> std::unique_ptr<T> {
            auto c = clone();
            return std::make_unique<T>(static_cast<T *>(c.release()));
        }
    };
}

#endif // ARGON_ICLONE_HPP