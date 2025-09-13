#include "InputSetter.hpp"
#include <ctime>
#include <fcntl.h>
#include <filesystem>
#include <libevdev/libevdev.h>
#include <string>

#include <unistd.h>
std::vector<dev_info>
InputSetter::ls_dev()
{
    std::vector<dev_info> devs;

    fs::path device_root("/dev/input/");
    
    for(const auto& dev : fs::directory_iterator(device_root)){
        if(!dev.is_character_file()){
            continue;
        }
        const std::string dev_path = dev.path();
        if(dev_path.find("event") == std::string::npos){
            continue;
        }

        int FD = open(dev_path.c_str(), O_RDONLY | O_NONBLOCK);

        if(FD < 0){
            continue;
        }
        libevdev* info = nullptr;

        if(libevdev_new_from_fd(FD, &info) == 0){
            const char* dev_name = libevdev_get_name(info);
            if(dev_name){
                dev_info tempInfo;
                tempInfo.dev_name = std::string(dev_name);
                tempInfo.loc = dev.path();
                tempInfo.has_relative_pos =
                libevdev_has_event_type(info, EV_REL) == 1;

                tempInfo.has_abs_pos =
                libevdev_has_event_type(info, EV_ABS) == 1;

                tempInfo.has_key =
                libevdev_has_event_type(info, EV_KEY) &&
                libevdev_has_event_code(info, EV_KEY, KEY_A) &&
                libevdev_has_event_code(info, EV_KEY, KEY_SPACE) &&
                libevdev_has_event_code(info, EV_KEY, KEY_ENTER);
                devs.push_back(tempInfo);
            }
            libevdev_free(info);
            close(FD);
        }
        else{
            close(FD);
            continue;
        }
    }
    return devs;
}

bool
EvWrap::Add(const dev_info& target)
{
    int FD = open(target.loc.c_str(), O_RDONLY | O_NONBLOCK);
    if(FD < 0){
        return false;
    }
    
    if(libevdev_new_from_fd(FD, &events[FD]) < 0){
        close(FD);
        events.erase(FD);
        return false;
    }
    libevdev_set_clock_id(events[FD], CLOCK_MONOTONIC_RAW);
    return true;
}

EvWrap::~EvWrap()
{
    for(auto& target : events){
        libevdev_free(target.second);
        close(target.first);
    }
}