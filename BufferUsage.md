# Introduction #

There is a ting::Buffer template class which encapsulates pointer to the memory buffer and the size of this buffer.
<br>
Why is it needed?<br>
<br>
When working with arrays in C++ we usually dealing with a plain pointer to the memory buffer and we should always be sure about the size of the allocated memory. Programming errors related to going out of memory buffer bounds are quite often and usually such bugs are tricky. They may exist in programs for some long time until one finds out that the program works incorrectly, or crashes. The ting::Buffer also holds the size of the memory buffer and it makes a boundary assertions when accessing the memory buffer. Of course, these assertions are only active when compiled in debug mode (DEBUG macro is defined).<br>
<br>
<br>
<br>
<br>
<h1>Header file</h1>

In order to use ting::Buffer or ting::StaticBuffer one needs to include the following header file:<br>
<pre><code>#include &lt;ting/Buffer.hpp&gt;<br>
</code></pre>



<br>
<h1>ting::Buffer</h1>

ting::Buffer is just a holder of a raw pointer to memory buffer and the size of this buffer. It does not do any memory management. So, if you have raw memory buffer pointer and you know its size, you can construct a ting::Buffer object out of these, but you still have to care about freeing the memory when it is not needed anymore (unless you are using ting::StaticBuffer or ting::Array):<br>
<pre><code>//here is our raw memory buffer pointer<br>
int *rawBuf = new int[143]; //we know that the size of the buffer is 143<br>
<br>
//create ting::Buffer object to use it further, for more safety.<br>
ting::Buffer&lt;int&gt; buf(rawBuf, 143);<br>
<br>
//later we can access buffer elements using operator[]<br>
buf[14] = 33;<br>
int a = buf[14]; //'a' is assigned value of 33<br>
<br>
//If we try to access out of buffer bounds, the assertion done inside the<br>
//ting::Buffer::operator[] will fail at runtime (if compiled in debug mode).<br>
buf[144] = 13; //assertion will fail here.<br>
<br>
//If we want to iterate through all of the buffer elements,<br>
//we can do like this:<br>
for(unsigned i = 0; i &lt; buf.Size(); ++i){<br>
    buf[i] = 14; //assign value of 14 to every element of the buffer.<br>
}<br>
<br>
//But it is more performance efficient to use Begin() and End() methods<br>
//which return pointers to first and after-last elements of the buffer:<br>
for(int* i = buf.Begin(); i != buf.End(); ++i){<br>
    *i = 14; //assign value of 14 to every element of the buffer.<br>
}<br>
</code></pre>



<br>
<h1>ting::StaticBuffer</h1>

For using ting::Buffer along with static buffers created on stack or as a member of some class there is a ting::StaticBuffer template. The ting::StaticBuffer is directly derived from ting::Buffer and, thus, inherits all its stuff.<br>
<br>
Here is how to use it:<br>
<pre><code>//create a static buffer object, e.g. on stack<br>
ting::StaticBuffer&lt;int, 143&gt; buf; //buffer of 143 int's<br>
<br>
//later we can use the buf as it would be ting::Buffer object.<br>
//...<br>
<br>
//In addition to that, we can create a copy of a static buffer,<br>
//using copy constructor, in which case the content of the buffer<br>
//will be copied to the newly created ting::StaticBuffer and<br>
//copy constructors will be called for every buffer element.<br>
<br>
ting::StaticBuffer&lt;int, 143&gt; copyBuf(buf);<br>
</code></pre>




<br>
<h1>ting::Array</h1>

To use ting::Buffer with dynamically allocated buffers there is a ting::Array template. It inherits ting::Buffer. For more info about ting::Array see the AutoPtrUsage wiki page.