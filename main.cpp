
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <libevdev/libevdev.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <sched.h>
#include <string>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include "IPCSHM.hpp"
#include "InputSetter.hpp"
#include <sys/wait.h>
#include "SocketServer.hpp"




int main(int argc, char** argv) {
    std::string socketPath = "/tmp/pdje_libevdev_socket_path.sock";
    SocketServer server(socketPath);
    
    server.Listen();
    
    std::cout << "socket server listening"<< std::endl;
    
    server.sudo_open("./INPUT_GETTER", socketPath);
    
    if(!server.Accept()){
        std::cerr << "failed to accept client" << std::endl;
    }

    std::string shmem_logger;
    IPCSharedMem<int, PROT_READ | PROT_WRITE> shared(&shmem_logger);
    
    if(!shared.MakeIPCSharedMemory("test_memfd", 10)){
        std::cerr << shmem_logger << std::endl;
    }
    
    int send_stat = server.send_fd_to_proc(shared);
    if(send_stat < 0){
        std::cerr << "failed to send fd, errno: " << errno << std::endl;
        return -7;
    }
    




    // InputSetter ed = InputSetter();
    // auto devs = ed.ls_dev();
    // auto evv = EvWrap();
    // for(const auto& info : devs){
    //     if(info.has_relative_pos){
    //         std::cout << "Rel Event=======================" << std::endl;
    //         std::cout << "dev name: " << info.dev_name << std::endl;
    //         std::cout << "dev path: " << info.loc.string() << std::endl;
    //     }
    //     if(info.has_abs_pos){
    //         std::cout << "Abs Event=======================" << std::endl;
    //         std::cout << "dev name: " << info.dev_name << std::endl;
    //         std::cout << "dev path: " << info.loc.string() << std::endl;
    //     }
    //     if(info.has_key){
    //         std::cout << "kb Event=======================" << std::endl;
    //         std::cout << "dev name: " << info.dev_name << std::endl;
    //         std::cout << "dev path: " << info.loc.string() << std::endl;
    //         evv.Add(info);
    //     }
    // }
    int* ip = shared.ptr;
    *ip = 10;
    bool temp;

    if(server.stop_proc(&temp)){
        std::cout << "server OK" << *ip << std::endl;
    }
    else{
        std::cout << "server off" << std::endl;
    }
    
    // ed.epoll_devs(evv);
    
    return 0;
}
 