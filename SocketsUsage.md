# Server side #

first of all we need to include corresponding header files
```
#include <ting/net/Lib.hpp>
#include <ting/net/TCPSocket.hpp>
#include <ting/net/TCPServerSocket.hpp>
```

After that we can start using the library
```
//In case of errors the ting::Socket::Exc exception is thrown
//So, enclose the network code into try/catch.
try{
    //Initialize the library by creating a singletone object.
    //It will deinitialize the library when the object
    //will be destroyed due to going out of scope.
    ting::net::Lib socketsLib;
    
    //create TCP server socket for listening
    ting::net::TCPServerSocket listenSock;
    
    //open listening socket and start listening
    listenSock.Open(80);//start listening on 80th port
    
    //Accept some connection
    ting::net::TCPSocket sock;
    while(!sock.IsValid()){
        sock = listenSock.Accept();
        //NOTE: it is a busy-loop which is not good.
        //Here we use it just to simplify the example.
    }
    
    //if we get here then new connection is accepted.
    
    //send some data
    ting::StaticBuffer<ting::u8, 4> data;
    data[0] = '0';
    data[1] = '1';
    data[2] = '2';
    data[3] = '4';
    sock.Send(data);
}catch(ting::net::Exc &e){
    std::cout << "Network error: " << e.What() << std::endl;
}
```


# Client side #

```
try{
    ting::net::Lib socketsLib;
    
    //create IP address for connecting
    ting::net::IPAddress ip("127.0.0.1", 80);//we will connect to localhost 80th port
    
    ting::net::TCPSocket sock;
    
    //connect to remote listening socket.
    //It is an asynchronous operation, so we will use WaitSet
    //to wait for its completion.
    sock.Open(ip);
    
    ting::WaitSet waitSet(1);
    waitSet.Add(sock, ting::Waitable::WRITE);
    waitSet.Wait(); //Wait for connection to complete.

    //receive data (expecting 4 bytes)
    ting::StaticBuffer<ting::u8, 4> data;
    unsigned bytesReceived = 0;
    while(bytesReceived < 4){
        bytesReceived += sock.Recv(data, bytesReceived);
    }
}catch(ting::net::Exc &e){
    std::cout << "Network error: " << e.What() << std::endl;
}
```

# Wait Sets #

note that it is also possible to use wait sets (ting::WaitSet) to check one or more sockets for activity. Using the wait sets is analogous to using the select() function from BSD sockets API.

First of all, include the wait set header file:
```
#include <ting/WaitSet.hpp>
```

To use wait set one needs to create the ting::WaitSet object:
```
//create a wait set for 10 objects at maximum
ting::WaitSet waitSet(10);
```

After creating the object one needs to add some sockets to the wait set:
```
ting::net::TCPSocket s1, s2, s3;

waitSet.Add(s1, ting::Waitable::READ);
waitSet.Add(s2, ting::Waitable::READ);
waitSet.Add(s3, ting::Waitable::READ);
```

Now we can check the sockets from socket set for read activity. To do this we use ting::WaitSet::Wait() function. It will return true if there are some sockets with activity or false if there is no any activity on sockets from socket set.
```
waitSet.Wait();
//There are some active sockets.
//This means that there are data available for reading from socket
//or remote socket has disconnected.

//... handle socket activity
```

After ting::WaitSet::Wait() returned we can check every socket for activity with ting::net::Socket::CanRead() function:
```
if(s1.CanRead()){
    //try to read some data from socket
    ting::StaticBuffer<ting::u8, 1024> buf;

    //after calling Recv() the read ready flag will be cleared
    unsigned len = sock.Recv(buf);
    if(len == 0){
       //0 received length means remote socket disconnection
       //Handle disconnection
       //...
    }
}

if(s2.CanRead()){
    //...
}
//etc.
```

# Error Handling #

Note that in case of errors a ting::Socket::Exc exception is thrown. Some of the examples above do not catch any exceptions just for code simplicity. In real life it is a good practice to catch exceptions ;-).


# UDP Socket server side #
```
#include <ting/net/UDPSocket.hpp>

try {
    ting::net::Lib socketsLib;
    ting::net::UDPSocket rcvSock;
    ting::net::IPAddress ip("127.0.0.1", 5060);
    rcvSock.Open(5060);
    while(true){
        ting::StaticBuffer<ting::u8, 1024> buf;
        rcvSock.Recv(buf, ip); 
    }
}catch(ting::net::Exc &e){
    std::cout << "Network error: " << e.What() << std::endl;
}
```

# UDP Sending side #

```
#include <ting/net/UDPSocket.hpp>

try {
    ting::net::Lib socketsLib;
    ting::net::UDPSocket sendSock;
    ting::net::IPAddress ip("127.0.0.1", 5060);
    sendSock.Open();

    ting::StaticBuffer<ting::u8, 4> data;
    data[0] = '0';
    data[1] = '1';
    data[2] = '2';
    data[3] = '4';
    sendSock.Send(data, ip);
}catch(ting::net::Exc &e){
    std::cout << "Network error: " << e.What() << std::endl;
}
```