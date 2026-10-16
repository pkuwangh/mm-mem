#ifndef MM_UTILS_COMMAND_LINE_H
#define MM_UTILS_COMMAND_LINE_H

#include <functional>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace mm_utils {

// Small integer-and-switch parser for benchmark options. Bound values must
// outlive the parser. Supports --name=value, --name value, -n value and -nVALUE.
class CommandLine {
  public:
    explicit CommandLine(const std::string& title = "") : title_(title) {}

    void group(const std::string& name) { group_ = name; }

    template <typename T, typename U>
    void value(const std::string& names, T* target, U default_value,
               const std::string& description) {
        static_assert(std::is_integral<T>::value && !std::is_same<T, bool>::value,
                      "CommandLine values must be integers");
        *target = static_cast<T>(default_value);
        add(names, description + " (default: " + std::to_string(*target) + ")",
            false, [target](const std::string& text) {
                const auto first = text.find_first_not_of("+-");
                if (text.empty() || first == std::string::npos || first > 1 ||
                    text.find_first_not_of("0123456789", first) != std::string::npos) {
                    throw std::invalid_argument("expected a decimal integer");
                }
                if (std::is_signed<T>::value) {
                    const auto parsed = std::stoll(text);
                    if (parsed < static_cast<long long>(std::numeric_limits<T>::min()) ||
                        parsed > static_cast<long long>(std::numeric_limits<T>::max())) {
                        throw std::out_of_range("integer out of range");
                    }
                    *target = static_cast<T>(parsed);
                } else {
                    if (text[0] == '-') {
                        throw std::invalid_argument("expected a nonnegative integer");
                    }
                    const auto parsed = std::stoull(text);
                    if (parsed > std::numeric_limits<T>::max()) {
                        throw std::out_of_range("integer out of range");
                    }
                    *target = static_cast<T>(parsed);
                }
            });
    }

    void flag(const std::string& names, bool* target, const std::string& description) {
        *target = false;
        add(names, description, true, [target](const std::string&) { *target = true; });
    }

    void parse(int argc, char** argv) const {
        std::vector<bool> seen(options_.size(), false);
        for (int i = 1; i < argc; ++i) {
            const std::string arg(argv[i]);
            if (arg == "--" && i + 1 == argc) {
                break;
            }
            const bool long_option = arg.compare(0, 2, "--") == 0;
            if (arg.size() < 2 || arg[0] != '-' || arg == "--") {
                throw std::invalid_argument("unexpected argument: " + arg);
            }
            const auto equal = arg.find('=');
            const std::string name = long_option ? arg.substr(2, equal == std::string::npos ? equal : equal - 2)
                                                 : arg.substr(1, 1);
            size_t index = 0;
            for (; index < options_.size(); ++index) {
                if (long_option ? options_[index].name == name : options_[index].alias == name) {
                    break;
                }
            }
            if (index == options_.size()) {
                throw std::invalid_argument("unknown option: " + arg);
            }
            const auto& option = options_[index];
            if (seen[index]) {
                throw std::invalid_argument("duplicate option: --" + option.name);
            }
            seen[index] = true;
            std::string text;
            const bool attached = long_option ? equal != std::string::npos : arg.size() > 2;
            if (option.is_flag) {
                if (attached) {
                    throw std::invalid_argument("option takes no value: --" + option.name);
                }
            } else if (attached) {
                text = long_option ? arg.substr(equal + 1) : arg.substr(2);
            } else {
                if (i + 1 == argc || std::string(argv[i + 1]).compare(0, 2, "--") == 0) {
                    throw std::invalid_argument("missing value: --" + option.name);
                }
                text = argv[++i];
            }
            try {
                option.assign(text);
            } catch (const std::exception& error) {
                throw std::invalid_argument("invalid value for --" + option.name + ": " + text + " (" + error.what() + ")");
            }
        }
    }

    void help(std::ostream& out) const {
        out << title_ << '\n';
        std::string previous_group;
        for (const auto& option : options_) {
            if (option.group != previous_group) {
                out << '\n' << option.group << ":\n";
                previous_group = option.group;
            }
            out << "  --" << option.name;
            if (!option.alias.empty()) {
                out << ", -" << option.alias;
            }
            out << (option.is_flag ? "" : " VALUE") << "\n      " << option.description << '\n';
        }
    }

  private:
    struct Option {
        std::string name, alias, description, group;
        bool is_flag;
        std::function<void(const std::string&)> assign;
    };
    std::string title_, group_;
    std::vector<Option> options_;

    void add(const std::string& names, const std::string& description, bool is_flag,
             const std::function<void(const std::string&)>& assign) {
        const auto comma = names.find(',');
        options_.push_back({names.substr(0, comma),
                            comma == std::string::npos ? "" : names.substr(comma + 1),
                            description, group_, is_flag, assign});
    }
};

} // namespace mm_utils

#endif
