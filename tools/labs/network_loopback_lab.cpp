// PROJECTALLMIGHT Phase 28 native loopback network witness: TCP byte stream framing and UDP datagram boundaries.
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct Fd{
    int value=-1;
    explicit Fd(int v=-1):value(v){}
    Fd(const Fd&)=delete;
    Fd& operator=(const Fd&)=delete;
    Fd(Fd&& o) noexcept:value(std::exchange(o.value,-1)){}
    Fd& operator=(Fd&& o) noexcept{
        if(this!=&o){ if(value>=0) ::close(value); value=std::exchange(o.value,-1); }
        return *this;
    }
    ~Fd(){ if(value>=0) ::close(value); }
};

void set_timeout(int fd){
    timeval tv{}; tv.tv_sec=5;
    require(::setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))==0,"setsockopt timeout failed");
}

std::vector<std::uint8_t> frame(const std::string& s){
    require(s.size()<=1024,"teaching frame too large");
    const std::uint32_t n=static_cast<std::uint32_t>(s.size());
    return {
        static_cast<std::uint8_t>(n>>24),static_cast<std::uint8_t>(n>>16),
        static_cast<std::uint8_t>(n>>8),static_cast<std::uint8_t>(n),
        s.begin(),s.end()
    };
}

class StreamDecoder{
public:
    void feed(const std::uint8_t* data,std::size_t n){
        buffer_.insert(buffer_.end(),data,data+n);
        for(;;){
            if(buffer_.size()<4) return;
            const std::uint32_t len=
                (static_cast<std::uint32_t>(buffer_[0])<<24)|
                (static_cast<std::uint32_t>(buffer_[1])<<16)|
                (static_cast<std::uint32_t>(buffer_[2])<<8)|
                 static_cast<std::uint32_t>(buffer_[3]);
            require(len<=1024,"declared frame exceeds maximum");
            if(buffer_.size()<4u+len) return;
            messages_.emplace_back(buffer_.begin()+4,buffer_.begin()+4+len);
            buffer_.erase(buffer_.begin(),buffer_.begin()+4+len);
        }
    }
    const std::vector<std::string>& messages() const { return messages_; }
private:
    std::vector<std::uint8_t> buffer_;
    std::vector<std::string> messages_;
};

void send_all(int fd,const std::uint8_t* p,std::size_t n){
    while(n){
        const ssize_t sent=::send(fd,p,n,0);
        if(sent<0) fail("send failed");
        require(sent>0,"send made no progress");
        p+=sent;
        n-=static_cast<std::size_t>(sent);
    }
}

sockaddr_in bound_loopback(int fd){
    sockaddr_in addr{};
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    addr.sin_port=0;
    require(::bind(fd,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))==0,"bind failed");
    socklen_t len=sizeof(addr);
    require(::getsockname(fd,reinterpret_cast<sockaddr*>(&addr),&len)==0,"getsockname failed");
    return addr;
}

void test_tcp_stream_framing(){
    Fd listener(::socket(AF_INET,SOCK_STREAM,0));
    require(listener.value>=0,"TCP listener socket failed");
    set_timeout(listener.value);
    const sockaddr_in addr=bound_loopback(listener.value);
    require(::listen(listener.value,1)==0,"listen failed");

    Fd client(::socket(AF_INET,SOCK_STREAM,0));
    require(client.value>=0,"TCP client socket failed");
    set_timeout(client.value);
    require(::connect(client.value,reinterpret_cast<const sockaddr*>(&addr),sizeof(addr))==0,"connect failed");

    Fd server(::accept(listener.value,nullptr,nullptr));
    require(server.value>=0,"accept failed");
    set_timeout(server.value);

    const auto a=frame("engine");
    const auto b=frame("renderer");
    std::vector<std::uint8_t> wire=a;
    wire.insert(wire.end(),b.begin(),b.end());

    // Deliberately split writes at positions unrelated to message boundaries.
    send_all(client.value,wire.data(),3);
    send_all(client.value,wire.data()+3,5);
    send_all(client.value,wire.data()+8,wire.size()-8);
    ::shutdown(client.value,SHUT_WR);

    StreamDecoder decoder;
    std::array<std::uint8_t,3> tiny{}; // force receive chunks unrelated to sender writes.
    for(;;){
        const ssize_t n=::recv(server.value,tiny.data(),tiny.size(),0);
        if(n==0) break;
        if(n<0) fail("recv failed");
        decoder.feed(tiny.data(),static_cast<std::size_t>(n));
    }
    require(decoder.messages()==std::vector<std::string>({"engine","renderer"}),
            "TCP stream decoder failed to reconstruct framed messages");
}

void test_udp_datagrams(){
    Fd receiver(::socket(AF_INET,SOCK_DGRAM,0));
    Fd sender(::socket(AF_INET,SOCK_DGRAM,0));
    require(receiver.value>=0&&sender.value>=0,"UDP socket creation failed");
    set_timeout(receiver.value);
    const sockaddr_in dst=bound_loopback(receiver.value);

    const std::string first="one";
    const std::string second="two-two";
    require(::sendto(sender.value,first.data(),first.size(),0,reinterpret_cast<const sockaddr*>(&dst),sizeof(dst))==
            static_cast<ssize_t>(first.size()),"UDP first send failed");
    require(::sendto(sender.value,second.data(),second.size(),0,reinterpret_cast<const sockaddr*>(&dst),sizeof(dst))==
            static_cast<ssize_t>(second.size()),"UDP second send failed");

    std::array<char,64> buf{};
    const ssize_t a=::recvfrom(receiver.value,buf.data(),buf.size(),0,nullptr,nullptr);
    require(a==static_cast<ssize_t>(first.size())&&std::string(buf.data(),static_cast<std::size_t>(a))==first,
            "UDP first datagram boundary/content mismatch");
    const ssize_t b=::recvfrom(receiver.value,buf.data(),buf.size(),0,nullptr,nullptr);
    require(b==static_cast<ssize_t>(second.size())&&std::string(buf.data(),static_cast<std::size_t>(b))==second,
            "UDP second datagram boundary/content mismatch");
}

} // namespace

int main(){
    try{
        test_tcp_stream_framing();
        test_udp_datagrams();
        std::cout<<"network_loopback_lab: PASS\n"
                 <<"proved: real POSIX loopback TCP byte-stream framing across arbitrary send/receive splits and UDP datagram boundary preservation\n"
                 <<"not proved: packet loss/reordering, congestion control behavior, MTU fragmentation, TLS, internet routing, Windows sockets, or production protocol security\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"network_loopback_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
