# Introduction #

**ting** offers implementation of a signal-slot callback mechanism. The implementation is based on C++ templates and C preprocessor. No additional tools or code generators needed.



<br>
<h1>Concept</h1>

Signal-slot is a technique of calling callback functions. The callback function may be an ordinary C-like function or function-member of some class. In latter case, it should be used in conjunction with object, for which the function-member is to be called.<br>
<br>
The <i><b>signal</b></i> is an object which holds list of callbacks it should call when needed.<br>
<br>
The <i><b>slot</b></i> is actually a callback function which is to be called.<br>
<br>
When adding a pointer to callback function to the callbacks list of a signal it is said that one connects a signal to a slot.<br>
<br>
When signal starts calling all the callbacks it has in its list, it is said that the signal is emitted.<br>
<br>
Signal is emitted by someone who wants to call the callbacks. In <b>ting</b> it is done by calling the Emit() method of Signal object.<br>
<br>
Signal may have arguments. The arguments are passed to the signal by callee as arguments to signal's Emit() method. Then, these arguments will be passed to every callback function (to every slot) in the signal's list.<br>
<br>
Signal can only be connected to those slots whose arguments match the ones of the signal. Although, it is also possible to connect the signal to slots with shorter list of arguments, in which case the rest of the signal arguments will not be passed to the slot function when the signal is emitted.<br>
<br>
It is possible to disconnect a signal from particular slot or from all slots it is currently connected to.<br>
<br>
<b>ting</b> signal-slot implementation is thread-safe in the sense that it is safe to connect, disconnect and emit signals from different threads. E.g. if one thread emits the signal and the other one tries to connect it to some slot.<br>
<br>
<br>
<br>
<br>
<h1>Header file</h1>
In order to use ting signal-slot, one needs to include this header file:<br>
<pre><code>#include &lt;ting/Signal.hpp&gt;<br>
</code></pre>



<br>
<h1>Signal objects</h1>

There is a <i>ting::Signal0</i> class which represents a signal with 0 arguments.<br>
And, there are template classes like:<br>
<i>ting::Signal1</i><br>
<i>ting::Signal2</i><br>
...<br>
<i>ting::Signal10</i><br>

which represent a signals with different number of arguments (from 1 to 10).<br>
<br>
To create a signal object with 0 arguments just do this:<br>
<pre><code>ting::Signal0 mySignal;<br>
</code></pre>

To create a signal, for example, with 3 arguments of types <code>int</code>, <code>char</code> and <code>int*</code>, do the following:<br>
<pre><code>ting::Signal3&lt;int, char, int*&gt; mySignalWithArgs;<br>
</code></pre>

The signal can then be emitted, whenever needed:<br>
<pre><code>mySignal.Emit();<br>
<br>
int intArray[2] = {13, 14};<br>
mySignalWithArgs.Emit(10, 'f', intArray);<br>
</code></pre>


<br>
<h1>Connecting signal to a simple Function slot</h1>

It is possible to connect the signal to any function (function slot) which matches its arguments list with signal's arguments list or has shorter<br>
arguments list than signal has but with matching argument types.<br>
<br>
Note, it is not important which value the function returns, return values are ignored when emitting the signal.<br>
<br>
Connecting slots without arguments:<br>
<pre><code>void MyFunctionSlotWith_0_Args(){<br>
    std::cout &lt;&lt; "Hello World!" &lt;&lt; std::endl;<br>
}<br>
<br>
mySignal.Connect(&amp;MyFunctionSlotWith_0_Args);<br>
mySignal.Emit();<br>
</code></pre>
As a result, the following will be printed:<br>
<pre><code>Hello World!<br>
</code></pre>


Connecting slots with arguments:<br>
<pre><code>void MyFunctionSlotWith_0_Args(){<br>
    std::cout &lt;&lt; "0 args func called" &lt;&lt; std::endl;<br>
}<br>
<br>
char MyFunctionSlotWith_1_Arg(int a){<br>
    std::cout &lt;&lt; "1 arg func called, a = " &lt;&lt; a &lt;&lt; std::endl;<br>
    return 'J';<br>
}<br>
<br>
void MyFunctionSlotWith_3_Args(int a, char c, int* ia){<br>
    std::cout &lt;&lt; "3 arg func called" &lt;&lt; std::endl;<br>
}<br>
<br>
int MyOverloadedFunctionSlot(int a){<br>
    std::cout &lt;&lt; "1 arg overloaded func called, a = " &lt;&lt; a &lt;&lt; std::endl;<br>
    return 3544;<br>
}<br>
<br>
bool MyOverloadedFunctionSlot(int a, char c){<br>
    std::cout &lt;&lt; "2 arg overloaded func called, a = " &lt;&lt; a &lt;&lt; std::endl;<br>
    return false;<br>
}<br>
<br>
//connect slots to signal<br>
<br>
mySignalWithArgs.Connect(&amp;MyFunctionSlotWith_0_Args);<br>
mySignalWithArgs.Connect(&amp;MyFunctionSlotWith_1_Arg);<br>
mySignalWithArgs.Connect(&amp;MyFunctionSlotWith_3_Args);<br>
<br>
//when connecting overloaded functions, it is necessary to indicate which one<br>
//you are exactly connecting by using C-style cast.<br>
mySignalWithArgs.Connect((int(*)(int)) &amp;MyOverloadedFunctionSlot);<br>
mySignalWithArgs.Connect((bool(*)(int, char)) &amp;MyOverloadedFunctionSlot);<br>
<br>
mySignalWithArgs.Emit(13, 'G', 0);//3rd argument is just a 0 pointer to int.<br>
</code></pre>
As a result, the following will be printed:<br>
<pre><code>0 args func called<br>
1 arg func called, a = 13<br>
3 arg func called<br>
1 arg overloaded func called, a = 13<br>
2 arg overloaded func called, a = 13<br>
</code></pre>

Note, that the above function slots have different types of return value. This is intentionally made to demonstrate that the return value does not matter and is ignored.<br>
<br>
<br>
<br>
<br>
<h1>Connecting signal to a Function-member slot</h1>
<pre><code>class MyClass{<br>
    int classData;<br>
public:<br>
    MyClass(int data) :<br>
            classData(data)<br>
    {}<br>
<br>
    void MemberSlotWith_3_Args(int a, char c, int* ia){<br>
        std::cout &lt;&lt; "3 arg method called, classData = " &lt;&lt; classData &lt;&lt; std::endl;<br>
    }<br>
<br>
    int OverloadedMemberSlot(int a){<br>
        std::cout &lt;&lt; "1 arg overloaded method called, a = " &lt;&lt; a &lt;&lt; std::endl;<br>
        return 3544;<br>
    }<br>
<br>
    bool OverloadedMemberSlot(int a, char c){<br>
        std::cout &lt;&lt; "2 arg overloaded method called, a = " &lt;&lt; a &lt;&lt; std::endl;<br>
        return false;<br>
    }<br>
};<br>
<br>
MyClass obj(14);<br>
<br>
mySignalWithArgs.Connect(&amp;obj, &amp;MyClass::MemberSlotWith_3_Args);<br>
<br>
//when connecting overloaded methods it is necessary to<br>
//indicate which one exactly you are connecting by using C-style cast<br>
mySignalWithArgs.Connect(<br>
        &amp;obj,<br>
        (int(MyClass::*)(int)) &amp;MyClass::OverloadedMemberSlot<br>
    );<br>
mySignalWithArgs.Connect(<br>
        &amp;obj,<br>
        (bool(MyClass::*)(int, char)) &amp;MyClass::OverloadedMemberSlot<br>
    );<br>
<br>
mySignalWithArgs.Emit(13, 'G', 0);//3rd argument is just a 0 pointer to int.<br>
</code></pre>

As a result, the following will be printed:<br>
<pre><code>3 arg method called, classData = 14<br>
1 arg overloaded method called, a = 13<br>
2 arg overloaded method called, a = 13<br>
</code></pre>


<br>
<h1>Connecting signal to a Function-member slot using ting::WeakRef</h1>

It is possible to connect signal to function-members slot using the <i>ting::WeakRef</i> instead of traditional C-like pointer. It is done in a very similar way, just pass a <i>ting::WeakRef</i> to the object instead of traditional pointer to the object as a first argument of Connect().<br>
<br>
<br>
<br>
<br>
<h1>Disconnecting</h1>

Disconnecting a signal from a particular slot is done similarly to connecting. Just use Disconnect() instead of Connect().<br>
<br>
Note, the Disconnect() method returns bool value indicating that the slot was actually disconnected or not. If it is <code>true</code> then the slot was disconnected. If it is <code>false</code> then the slot was not disconnected, possibly, due to it was not connected before to this signal.<br>
<br>
<br>
<br>
<br>
<h1>Checking if slot is connected to a signal</h1>

There is a family of SignalX::IsConnected() methods. The method arguments are quite similar to SignalX::Connect() ones. The IsConnected() method checks if the slot is already connected to the given signal and returns true if it is connected, or false otherwise.