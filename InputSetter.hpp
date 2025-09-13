#pragma once

#include <utility>
#include <vector>
#include <string>
#include <filesystem>

#include <libevdev/libevdev.h>

namespace fs = std::filesystem;
#include <unordered_map>

struct dev_info {
    std::string dev_name;
    bool has_relative_pos;
    bool has_abs_pos;
    bool has_key;
    fs::path loc;
};

class EvWrap{
private:
public:
    std::unordered_map<int, libevdev*> events;
    
    EvWrap() = default;
    bool Add(const dev_info& target);
    ~EvWrap();
};


class InputSetter{

private:
public:
    std::vector<dev_info> ls_dev();
    void epoll_devs();
    void parse_event();
    InputSetter() = default;
    ~InputSetter() = default;
};