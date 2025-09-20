#pragma once

#include <cstddef>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "nlohmann/json_fwd.hpp"
#include "spawn.h"
#include "IPCSHM.hpp"
#include <string>
#include "InputSetter.hpp"
#include <nlohmann/json.hpp>
#include <arpa/inet.h>

enum SocketTransmission{
    GET_DEVICE = 0,
    SET_DEVICE,
    SET_INPUT_LOOP_ARG
};
using nj = nlohmann::json;
class SocketServer {
private:
    int socket_FD;
    int client_FD;
    pid_t proc_pid;
    std::string __socketPath;
    nj jparser;
    int sendByte(const void* data, size_t len);
    int readByte(void* data, size_t len);
public:
    bool Listen();
    bool Accept();
    bool sudo_open(const std::string& exec_path, const std::string& arg);
    
    template<typename T, int MEM_PROT_FLAG>
    int send_fd_to_proc(const IPCSharedMem<T,MEM_PROT_FLAG>& shared_IPC_mem);

    std::vector<dev_info> request_device_list(std::string& ErrOut);
    bool stop_proc(bool* shared_mem_stop_flag);

    SocketServer(const std::string& socket_file_path);
    ~SocketServer();
};


template<typename T, int MEM_PROT_FLAG>
int
SocketServer::send_fd_to_proc(const IPCSharedMem<T,MEM_PROT_FLAG>& shared_IPC_mem){
        struct msghdr msg = {0};
        //data trans
        struct iovec iov = {
            .iov_base = (void*)"F",    // 실제 payload 데이터 (필수는 아님)
            .iov_len  = 1
        };
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        //end dummy data trans

        char cmsgbuf[CMSG_SPACE(sizeof(int))]; // ancillary buffer
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = sizeof(cmsgbuf);

        struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type  = SCM_RIGHTS;
        cmsg->cmsg_len   = CMSG_LEN(sizeof(int));
        
        memcpy(
            CMSG_DATA(cmsg), 
            &shared_IPC_mem.FD, 
            sizeof(int));
        return sendmsg(client_FD, &msg, 0);
    }