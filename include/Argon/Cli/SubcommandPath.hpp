#ifndef ARGON_SUBCOMMAND_PATH_HPP
#define ARGON_SUBCOMMAND_PATH_HPP

#include <string>
#include <vector>

namespace Argon {
    struct SubcommandPath {
        std::vector<std::string> path;

        auto empty() const -> bool {
            return path.empty();
        }

        auto push(std::string_view cmd) {
            path.emplace_back(cmd);
        }

        auto pop() {
            path.pop_back();
        }

        auto toString() const -> std::string {
            std::string s = "";
            if (path.empty()) return s;
            s += path.front();
            for (size_t i = 1; i < path.size(); ++i) {
                s += " " + path[i];
            }
            return s;
        }
    };
}

#endif // ARGON_SUBCOMMAND_PATH_HPP