#include "SocketServer.hpp"
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>


SocketServer::SocketServer(const std::string& socket_file_path)
{
    unlink(socket_file_path.c_str());
    __socketPath = socket_file_path;
    socket_FD = socket(AF_UNIX, SOCK_STREAM, 0);
    
}

bool
SocketServer::Listen()
{
    sockaddr_un address_temp{};
    address_temp.sun_family = AF_UNIX;

    if(
        bind(socket_FD,
        reinterpret_cast<sockaddr*>(&address_temp),
        sizeof(address_temp)
        ) < 0
    ){
        return false;
    }

    if(listen(socket_FD, 1) < 0){
        return false;
    }
    return true;
}


bool
SocketServer::Accept()
{
    client_FD = accept(socket_FD, nullptr, nullptr);
    if(client_FD < 0){
        return false;
    }
    return true;
}


SocketServer::~SocketServer()
{
    close(client_FD);
    close(socket_FD);
    unlink(__socketPath.c_str());
    
}

bool
SocketServer::stop_proc(bool* stop_flag)
{
    *stop_flag = false;
    int stat = -1;
    waitpid(proc_pid, &stat, 0);
    return stat == 0;
}