#include <cerrno>
#include <iostream>
#include <libevdev/libevdev.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <string>
#include <sys/epoll.h>

#include "InputSetter.hpp"

int main(int argc, char** argv) {
    InputSetter ed = InputSetter();
    auto devs = ed.ls_dev();
    auto evv = EvWrap();
    for(const auto& info : devs){
        if(info.has_relative_pos){
            std::cout << "Rel Event=======================" << std::endl;
            std::cout << "dev name: " << info.dev_name << std::endl;
            std::cout << "dev path: " << info.loc.string() << std::endl;
        }
        if(info.has_abs_pos){
            std::cout << "Abs Event=======================" << std::endl;
            std::cout << "dev name: " << info.dev_name << std::endl;
            std::cout << "dev path: " << info.loc.string() << std::endl;
        }
        if(info.has_key){
            std::cout << "kb Event=======================" << std::endl;
            std::cout << "dev name: " << info.dev_name << std::endl;
            std::cout << "dev path: " << info.loc.string() << std::endl;
            evv.Add(info);
        }
    }
    
    int epfd = epoll_create1(EPOLL_CLOEXEC);
    
    
    for(const auto& dev : evv.events){
        epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = dev.first;
        epoll_ctl(epfd, EPOLL_CTL_ADD, dev.first, &ev);
    }
    std::cout << "Ready" << std::endl;
    bool flag_loop = true;
    while(flag_loop){
        epoll_event out_events[64];
        int n = epoll_wait(epfd, out_events, 64, -1);
        std::cout << "trigged" << n << std::endl;
        for(int i =0; i<n; ++i){
            input_event now_ev;
            bool flag_inner_loop = true;
            while(flag_inner_loop){
                CheckEvent:
                int status = libevdev_next_event(
                    evv.events[out_events[i].data.fd], 
                    LIBEVDEV_READ_FLAG_NORMAL, &now_ev);
                
                switch (status) {
                case LIBEVDEV_READ_STATUS_SUCCESS:{
                    if(now_ev.type == EV_KEY &&
                        now_ev.code == KEY_A &&
                        now_ev.value == 1){
                            flag_loop = false;
                            std::cout << "pressed";
                        }
                    goto CheckEvent;
                }
                case LIBEVDEV_READ_STATUS_SYNC:{

                    SyncEvent:
                    int sync_status = libevdev_next_event(
                        evv.events[out_events[i].data.fd], 
                        LIBEVDEV_READ_FLAG_SYNC, 
                        &now_ev);
                    
                    
                    switch (sync_status) {
                    case LIBEVDEV_READ_STATUS_SUCCESS:{
                        goto SyncEvent;
                    }
                    case -EAGAIN:{
                        goto CheckEvent;
                        
                    }
                    default:
                        break;
                    }
                    break;
                }
                case -EAGAIN:{
                    goto outofloop;
                    break;
                }
                case -ENODEV:{
                    //delete device from epoll
                    break;
                }
                default:{
                    break;
                }
                    
                }
                
            }
            outofloop:
        }
    }
    return 0;
}
    // std::string dev = (argc > 1) ? argv[1] : "/dev/input/event0";

    // int fd = open(dev.c_str(), O_RDONLY | O_NONBLOCK);
    // if (fd < 0) {
    //     std::perror(("open " + dev).c_str());
    //     return 1;
    // }

    // libevdev* devh = nullptr;
    // int rc = libevdev_new_from_fd(fd, &devh);
    // if (rc < 0) {
    //     std::cerr << "libevdev_new_from_fd: " << strerror(-rc) << "\n";
    //     close(fd);
    //     return 1;
    // }

    // std::cout << "Device: " << libevdev_get_name(devh)
    //           << " bustype=" << libevdev_get_id_bustype(devh)
    //           << " vendor="  << libevdev_get_id_vendor(devh)
    //           << " product=" << libevdev_get_id_product(devh) << "\n";

    // // 필요하면 전용 점유(그랩) — 다른 사용자 공간으로 이벤트 전달을 막음(주의)
    // // rc = libevdev_grab(devh, LIBEVDEV_GRAB);
    // // if (rc) std::cerr << "grab failed: " << strerror(-rc) << "\n";

    // pollfd pfd{fd, POLLIN, 0};

    // while (true) {
    //     int pr = poll(&pfd, 1, /*timeout_ms*/ 3000);
    //     if (pr < 0) {
    //         if (errno == EINTR) continue;
    //         std::perror("poll");
    //         break;
    //     }
    //     if (pr == 0) {
    //         // timeout — 필요시 continue
    //         continue;
    //     }

    //     // 한 번에 여러 이벤트가 들어올 수 있으니 반복 호출
    //     while (true) {
    //         input_event ev;
    //         rc = libevdev_next_event(devh, LIBEVDEV_READ_FLAG_NORMAL, &ev);

    //         if (rc == 0) {
    //             // 정상 이벤트
    //             if (ev.type == EV_KEY) {
    //                 // 키보드/마우스 버튼: value=1(press), 0(release), 2(auto-repeat)
    //                 std::cout << "KEY code=" << ev.code << " value=" << ev.value << "\n";
    //             } else if (ev.type == EV_REL) {
    //                 // 마우스 상대 이동 (REL_X, REL_Y 등)
    //                 std::cout << "REL code=" << ev.code << " delta=" << ev.value << "\n";
    //             } else if (ev.type == EV_ABS) {
    //                 // 터치/스틱 절대값
    //                 std::cout << "ABS code=" << ev.code << " val=" << ev.value << "\n";
    //             } else if (ev.type == EV_MSC) {
    //                 // 스캔코드 등 (MSC_SCAN)
    //                 std::cout << "MSC code=" << ev.code << " val=" << ev.value << "\n";
    //             } else if (ev.type == EV_SYN) {
    //                 // 프레임 경계
    //                 // std::cout << "SYN\n";
    //             }
    //         } else if (rc == -EAGAIN) {
    //             // 현재 읽을 이벤트 없음 — 다음 poll로
    //             break;
    //         } else if (rc == LIBEVDEV_READ_STATUS_SYNC || rc == O_SYNC) {
    //             // 드롭 발생: SYNC 모드로 상태 재동기화 필요
    //             // 커널 링버퍼 오버런 시 발생. 아래처럼 모두 비울 때까지 읽어줍니다.
    //             while (true) {
    //                 rc = libevdev_next_event(devh, LIBEVDEV_READ_FLAG_SYNC, &ev);
    //                 if (rc == 0) {
    //                     // SYNC 보정 이벤트 스트림
    //                     // 필요시 상태 갱신 처리
    //                 } else if (rc == -EAGAIN) {
    //                     // SYNC 끝 — 정상 모드 복귀
    //                     break;
    //                 } else {
    //                     std::cerr << "sync read err: " << strerror(-rc) << "\n";
    //                     break;
    //                 }
    //             }
    //         } else {
    //             std::cerr << "read err: " << strerror(-rc) << "\n";
    //             break;
    //         }
    //     }
    // }

    // // 필요시 해제
    // // libevdev_grab(devh, LIBEVDEV_UNGRAB);
    // libevdev_free(devh);
    // close(fd);
//     return 0;
// }
