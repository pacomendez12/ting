# Creating a Thread #
first of all we need to include **ting** header to use the library
```
#include <ting/mt/Thread.hpp>
```

After that we can start using the library. To create a thread one needs to derive a new class representing her thread from _ting::mt::Thread_ base class and override its _ting::mt::Thread::Run()_ method
```
class MyThread : public ting::mt::Thread{
    //define some variables specific to your thread,
    //in this example we have two ints: a and b
    int a;
    int b;

public:
    //override
    void Run(){
        //do some thread actions
    };
};
```
After we have our thread class defined we can create an object of this class and start the thread execution by calling _ting::mt::Thread::Start()_ method on it
```
MyThread thr; //create an object on the stack for simplicity
thr.Start();//start thread execution
```
After starting the thread we can wait for it to finish its execution by calling _ting::mt::Thread::Join()_ method on the thread object. After this call we can surely say that thread has finished its execution
```
thr.Join();
```



# Messages and Message Queues #
To use Messages/Queues facility along with threads it is recommended to use _ting::mt::MsgThread_ class as a base class instead of simple _ting::mt::Thread_. This is because _ting::mt::MsgThread_ class already contains a message queue object (_ting::mt::Thread::queue_). Note, that _ting::mt::MsgThread_ also contains a _quitFlag_ boolean variable, just for convenience. See examples below for usage hints.

From now on we will assume that our sample _MyThread_ class is derived from _ting::mt::MsgThread_
```
class MyThread : public ting::mt::MsgThread{
    int a;
    int b;

public:
    //override
    void Run(){
        //do some thread actions
    };
};
```

Thus the thread can receive and handle messages from other threads. To make it possible, one needs to implement the message handling loop in the threads _ting::mt::Thread::Run()_ method.

Typical implementation of the _ting::mt::Thread::Run()_ method which handles messages looks as follows
```
void MyThread::Run(){
    while(!this->quitFlag){
        ting::Ptr<ting::mt::Message> m = this->queue.GetMsg();
        m->Handle();
    }
};
```
This example uses _ting::Queue::GetMsg()_ to retrieve messages from the queue, _GetMsg()_ blocks execution if there are no messages on the queue until some message arrives.

It is also possible to use _ting::Queue::PeekMsg()_ which does not block execution, then typical implementation of _Run()_ method would look as follows

```
void MyThread::Run(){
    while(!this->quitFlag){
        //handle all messages on the queue
        while(ting::Ptr<ting::mt::Message> m = this->queue.PeekMsg()){
            m->Handle();
        }
        //do some actions to be done every cycle
        //...
    }
};
```

Note that each thread object has a _quitFlag_ volatile member variable which serves as a convenient indicator to the thread showing when the thread should finish its execution.

Note that _ting::mt::QuitMessage_ will set the _quitFlag_ when it is handled.

## Creating your own message ##
A _Message_ is a mean to ask some thread to execute some code fragment.
To do this one needs to create his/her own message (implementing its handler, i.e define the code fragment we want other to execute) and send it to the threads queue.

Let's define our own message class deriving it from _ting::mt::Message_
```
class MyMessage : public ting::mt::Message{
    MyThread *myThr;
public:
    MyMessage(MyThread *mt) :
            myThr(mt)
    {
        //assert that this->myThr is not 0.
        //...
    };

    //override
    void Handle(){
        //implement handler for this message
        this->myThr->a = 10;
        this->myThr->b = 20;
    };
};
```

Now, we can send this message to the thread's queue
```
//Create and start the thread we're going to send the message to
MyThread thread;
thread.Start();

//create the message
ting::Ptr<ting::mt::Message> msg( new MyMessage(&thread) );

//send the message
thread.PushMessage(msg);
```
Note, that our _Handle()_ method accesses private variables of the thread, so one should probably add a friend declaration to the _MyThread_ class to indicate that _MyMessage_ is a friend of _MyThread_ class and can access its private variables. This will also ideologically indicate that _MyThread_ can handle messages of type _MyMessage_.