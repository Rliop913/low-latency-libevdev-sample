#include "InputSetter.hpp"
#include <fcntl.h>
#include <filesystem>
#include <libevdev/libevdev.h>
#include <string>


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

        if(libevdev_new_from_fd(FD, &info) >= 0){
            const char* dev_name = libevdev_get_name(info);
            libevdev_has_event_type(info, EV_REL);
            libevdev_has_event_type(info, EV_ABS);
            libevdev_has_event_type(info, EV_KEY);

            
            if(dev_name){
                dev_info tempInfo;
                tempInfo.dev_name = std::string(dev_name);
                tempInfo.path = dev_path;
                devs.push_back(tempInfo);
            }
        }
    }
    return devs;
}