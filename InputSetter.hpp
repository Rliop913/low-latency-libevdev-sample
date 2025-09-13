#pragma once

#include <utility>
#include <vector>
#include <string>
#include <filesystem>

#include <libevdev/libevdev.h>

namespace fs = std::filesystem;

struct dev_info {
    std::string dev_name;
    std::string path;
};

class InputSetter{

private:
    std::vector<dev_info> ls_dev();
    bool open_dev();
    void epoll_devs();
    void parse_event();
public:
    InputSetter() = default;
    ~InputSetter() = default;
};