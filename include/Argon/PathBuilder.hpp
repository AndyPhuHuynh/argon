#ifndef ARGON_PATH_BUILDER_HPP
#define ARGON_PATH_BUILDER_HPP

#include <string>
#include <vector>

namespace Argon {
    struct PathBuilder {
        std::vector<std::string> path;

        [[nodiscard]] auto empty() const -> bool {
            return path.empty();
        }

        auto push(std::string_view cmd) {
            path.emplace_back(cmd);
        }

        auto pop() {
            path.pop_back();
        }

        [[nodiscard]] auto toString(const std::string_view delimiter) const -> std::string {
            std::string s;
            if (path.empty()) return s;
            s += path.front();
            for (size_t i = 1; i < path.size(); ++i) {
                s += delimiter;
                s += path[i];
            }
            return s;
        }
    };
}

#endif // ARGON_PATH_BUILDER_HPP