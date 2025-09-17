#include "InputSetter.hpp"
#include <cerrno>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <filesystem>
#include <libevdev/libevdev.h>
#include <linux/input.h>
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


void
InputSetter::use_event(const input_event& evtrig)
{
    if(evtrig.type == EV_KEY &&
        evtrig.code == KEY_A &&
        evtrig.value == 1){
            std::cout << "pressed";
        }
    return;
}



bool
InputSetter::drain_events(const int epFD, int FD, libevdev* evdev)
{
    input_event now_ev;
    auto flag = LIBEVDEV_READ_FLAG_NORMAL;
    bool inSync = false;
    int state = 0;
    while(true){
        flag = inSync ? LIBEVDEV_READ_FLAG_SYNC : LIBEVDEV_READ_FLAG_NORMAL;
        state = libevdev_next_event(evdev, flag, &now_ev);

        switch (state) {
        case LIBEVDEV_READ_STATUS_SUCCESS:
            use_event(now_ev);
            continue;
        
        case LIBEVDEV_READ_STATUS_SYNC:
            inSync = true;
            continue;
            
        case -EAGAIN:
            if(inSync){
                inSync = false;
                continue;
            }
            else{
                return true;
            }
        case -ENOMEM:{
            timespec sleepTime{.tv_nsec=300000};
            nanosleep(&sleepTime, nullptr);
            continue;
        }

        case -ENODEV:
        [[fallthrough]];
        default:
            epoll_ctl(epFD, EPOLL_CTL_DEL, FD, nullptr);
            return false;
            break;
            
        }   
    }
}
    


void
InputSetter::epoll_devs(EvWrap& targets)
{
    int epfd = epoll_create1(EPOLL_CLOEXEC);
    
    
    for(const auto& dev : targets.events){
        epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = dev.first;
        epoll_ctl(epfd, EPOLL_CTL_ADD, dev.first, &ev);
    }
    sched_param sp{};
    sp.sched_priority = 70; // 예: 70~90
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp) != 0) {
        std::cerr << "pthread_setschedparam failed: " << stderr << std::endl;
        return;
    }
    while(true){
        epoll_event out_events[64];
        int n = epoll_wait(epfd, out_events, 64, -1);
        std::cout << "trigged" << n << std::endl;
        
        for(int i =0; i<n; ++i){
            drain_events(epfd, out_events[i].data.fd, targets.events[out_events[i].data.fd]);
        }
    }
    return;
    
}