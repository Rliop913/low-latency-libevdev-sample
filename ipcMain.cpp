#include <cstddef>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>


int recv_fd(int sock) {
    msghdr msg = {0};

    char m_buffer[1];
    iovec io = {
        .iov_base = m_buffer,
        .iov_len = sizeof(m_buffer) };
    
    msg.msg_iov = &io; 
    msg.msg_iovlen = 1;

    char c_buffer[CMSG_SPACE(sizeof(int))];


    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);

    if (recvmsg(sock, &msg, 0) <= 0) return -1;

    cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        int fd; 
        memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
        return fd; // 성공: 상대가 보낸 FD 획득
    }
    return -1;
}

int main(int argc, char** argv){
    if(argc < 1){
        return -1;
    }
    std::cout << "spawned!!!" << argv[1] << std::endl;
    
    std::string socket_path(argv[1]);
    
    int socket_int = socket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    if(connect(socket_int,
        reinterpret_cast<sockaddr*>(&address), 
        sizeof(address)) < 0){
            std::cerr << "connection failed" << std::endl;
            return -1;
        }
    
        
    int FD = recv_fd(socket_int);
    
    if(FD < 0){
        std::cerr << "failed to get memFD" << std::endl;
        return -2;
    }
    size_t SZ = sizeof(int);
    void* p = mmap(nullptr, SZ, PROT_READ | PROT_WRITE, MAP_SHARED, FD, 0);
    if(p == MAP_FAILED){
        std::cerr << "failed to read mmap" << std::endl;
        return -3;
    }
    int* ip = (int*)p;
    while(*ip != 10){
        std::cout << "waiting..." << std::endl;
    }
    *ip = 100;
    munmap(p, SZ);
    close(FD);
    close(socket_int);

    for(auto i=0; i<argc; ++i){
        std::cout << i << ", " <<argv[i]<<std::endl;
    }
    return 0;
}