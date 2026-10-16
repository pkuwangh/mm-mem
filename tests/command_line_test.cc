#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "common/command_line.h"

void require(bool condition) {
    if (!condition) {
        throw std::runtime_error("command-line test failed");
    }
}

struct Fixture {
    uint32_t count;
    uint64_t size;
    int32_t delay;
    bool verbose;
    mm_utils::CommandLine options{"Test benchmark"};

    Fixture() {
        options.group("Generic options");
        options.value("count,n", &count, 4, "thread count");
        options.value("size,b", &size, 1024, "region size");
        options.value("delay,d", &delay, 0, "delay");
        options.flag("verbose,v", &verbose, "verbose output");
    }

    void parse(std::vector<std::string> args) {
        args.insert(args.begin(), "test");
        std::vector<char*> argv;
        for (auto& arg : args) {
            argv.push_back(&arg[0]);
        }
        options.parse(argv.size(), argv.data());
    }
};

int main() {
    Fixture defaults;
    defaults.parse({});
    require(defaults.count == 4 && defaults.size == 1024 && defaults.delay == 0 && !defaults.verbose);
    Fixture long_options;
    long_options.parse({"--count=4294967295", "--size", "18446744073709551615", "--delay=-2147483648", "--verbose"});
    require(long_options.count == UINT32_MAX && long_options.size == UINT64_MAX &&
            long_options.delay == INT32_MIN && long_options.verbose);
    Fixture short_options;
    short_options.parse({"-n2", "-b", "4096", "-d2147483647", "-v", "--"});
    require(short_options.count == 2 && short_options.size == 4096 &&
            short_options.delay == INT32_MAX && short_options.verbose);
    Fixture plus;
    plus.parse({"--count", "+2", "--size=0", "-d", "-1"});
    require(plus.count == 2 && plus.size == 0 && plus.delay == -1);

    const std::vector<std::vector<std::string>> invalid = {
        {"--unknown"}, {"positional"}, {"--", "positional"},
        {"--count"}, {"--count", "--verbose"}, {"--count="},
        {"--count", "-1"}, {"--count=4294967296"},
        {"--size=18446744073709551616"}, {"--delay=2147483648"},
        {"--delay=-2147483649"}, {"--count=2junk"}, {"--count=1.5"},
        {"--count= 2"}, {"--count=++2"}, {"--count=+-2"},
        {"--count=+"}, {"--count=1", "-n2"},
        {"--verbose=true"}, {"-v1"}, {"--verbose", "-v"},
    };
    for (const auto& args : invalid) {
        Fixture fixture;
        bool rejected = false;
        try {
            fixture.parse(args);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        require(rejected);
    }
    std::ostringstream help;
    defaults.options.help(help);
    require(help.str().find("Test benchmark") != std::string::npos);
    require(help.str().find("Generic options") != std::string::npos);
    require(help.str().find("--count, -n VALUE") != std::string::npos);
    require(help.str().find("default: 4") != std::string::npos);
    std::cout << "Command-line parser checks passed\n";
}
