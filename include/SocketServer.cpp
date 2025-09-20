#include "SocketServer.hpp"
#include "InputSetter.hpp"
#include <bits/types/error_t.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
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

int
SocketServer::sendByte(const void* data, size_t len)
{
    const char* p = reinterpret_cast<const char*>(data);
    while(len > 0){
        auto sended_bytes = send(client_FD, p, len, MSG_NOSIGNAL);
        
        if(sended_bytes < 0){
            if(errno == EINTR) continue; // ignore signal
            if(errno == EAGAIN) continue; // nonblock 
            return -1;
        }
        else if(sended_bytes == 0){
            return -2;
        }
        p += sended_bytes;
        len -= sended_bytes;
    }
    return len;
}


int
SocketServer::readByte(void* data, size_t len)
{
    char* p = reinterpret_cast<char*>(data);
    while(len > 0){
        auto readed_bytes = recv(client_FD, p, len, 0);
        
        if(readed_bytes < 0){
            if(errno == EINTR) continue; // ignore signal
            if(errno == EAGAIN) continue; // nonblock 
            return -1;
        }
        else if(readed_bytes == 0){
            return -2;
        }
        p += readed_bytes;
        len -= readed_bytes;
    }
    return len;
}

std::vector<dev_info>
SocketServer::request_device_list(std::string& ErrOut)
{
    jparser.clear();
    jparser["HEAD"] = "GET_DEV";
    auto dumped_bytes = jparser.dump();
    auto data_len =dumped_bytes.size();
    if(data_len > std::numeric_limits<uint32_t>::max()){
        ErrOut = "data length exceeded unsigned int 32 max length";
        return {};
    }
    if(data_len == 0){
        ErrOut = "data length is zero";
        return {};
    }
    auto net_len =  htonl(static_cast<uint32_t>(data_len));

    if(sendByte(&net_len, sizeof(uint32_t)) < 0){
        ErrOut = "failed to send length byte";
        return {};
    }

    if(sendByte(dumped_bytes.data(), data_len) < 0){
        ErrOut = "failed to send json byte";
        return {};
    }

    jparser.clear();
    
    if(readByte(&net_len, sizeof(uint32_t)) < 0){
        ErrOut = "failed to read length byte";
        return {};
    }

    std::string dev_list;
    dev_list.resize(ntohl(net_len));

    if(readByte(dev_list.data(), dev_list.size()) < 0){
        ErrOut = "failed to read json byte";
        return {};
    }

    try {
        jparser = nj::parse(dev_list);
    } catch (std::exception& e) {
        ErrOut = "failed to parse json data";
        return {};
    }
    if(jparser.find("HEAD") == jparser.end() || jparser.find("BODY") == jparser.end()){
        ErrOut = "failed to get valid json key";
        return {};
    }

    if(jparser["HEAD"] != "GET_DEV"){
        ErrOut = "failed to get valid HEAD, got: " + jparser["HEAD"].get<std::string>();
        return {};
    }

    auto Devices = jparser["BODY"];
    std::vector<dev_info> Dlist;
    for(auto& dev : Devices){
        if(dev.find("NAME") == dev.end() || dev.find("TYPE") == dev.end()){
            continue;
        }
        dev_info di;
        di.dev_name = dev["NAME"];
        di.dev_type = dev["TYPE"];
        Dlist.push_back(di);
    }
    return Dlist;    
}
