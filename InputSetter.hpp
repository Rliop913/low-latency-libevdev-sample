#pragma once

#include <linux/input.h>
#include <sched.h>
#include <utility>
#include <vector>
#include <string>
#include <filesystem>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <libevdev/libevdev.h>
#include <numa.h>
#include <numaif.h>
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
    bool drain_events(const int epFD, int FD, libevdev* evdev);
    void use_event(const input_event& evtrig);
    void remove_device();
    public:
    std::vector<int> sample;
    std::vector<dev_info> ls_dev();
    void epoll_devs(EvWrap& targets);
    void parse_event();
public:
    static int cpu_valid_check(int& cpu_number);
    static bool set_cpu(std::string& ErrOut, int cpu_number = 2);
    InputSetter(std::string& ErrOut){
        mlockall(MCL_CURRENT | MCL_FUTURE);
    }
    ~InputSetter(){
        munlockall();
    }
};