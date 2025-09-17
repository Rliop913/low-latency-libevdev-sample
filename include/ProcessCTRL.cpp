#include "SocketServer.hpp"


extern char **environ;

bool
SocketServer::sudo_open(const std::string& exec_path, const std::string& arg)
{
    char* pkexec_args[] = {
        (char*)"pkexec",
        (char*)exec_path.c_str(),
        (char*)arg.c_str(),
        nullptr
    };
    char* sudo_args[] = {
        (char*)"sudo",
        (char*)exec_path.c_str(),
        (char*)arg.c_str(),
        nullptr
    };
    

    if((getenv("DISPLAY") || getenv("WAYLAND_DISPLAY")) && access("/usr/bin/pkexec",X_OK) == 0){
        int spawn_stat = posix_spawn(&proc_pid, "/usr/bin/pkexec", nullptr, nullptr, pkexec_args, environ);
        if(spawn_stat == 0){
            return true;
        }
    }
    else{
        int spawn_stat = posix_spawn(&proc_pid, "/usr/bin/sudo", nullptr, nullptr, sudo_args, environ);
        if(spawn_stat == 0){
            return true;
        }
    }
    return false;
}