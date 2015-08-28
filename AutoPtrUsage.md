# Introduction #

**ting** offers its own implementation of auto pointer. It is very similar to _std::auto\_ptr_, but have some improvements, like automatic conversion to bool-like type.
Intention of auto pointers is to prevent possible memory leaks and automatically free memory occupied by object when auto pointer goes out of scope.

**ting** auto pointer is implemented as _ting::Ptr_ class. Array auto pointer implemented as _ting::Array_ class.



<br>
<h1>Header file</h1>

In order to use <i>ting::Ptr</i> or <i>ting::Array</i> one needs to include the following header file:<br>
<pre><code>#include &lt;ting/Ptr.hpp&gt; //for ting::Ptr<br>
#include &lt;ting/Array.hpp&gt; //for ting::Array<br>
</code></pre>



<br>
<h1>ting::Ptr usage</h1>

The main concept of auto pointer is that only one auto pointer owns the object. It is allowed to assign auto pointers to each other, but, in that case the object owning will be transferred from one auto pointer to another. This means that when assigning autoptr#1 to autoptr#2 the autoptr#1 will become invalid and autoptr#2 will hold the reference to the object.<br>
<br>
Example:<br>
<pre><code>class MyClass{<br>
public:<br>
    int a;<br>
};<br>
<br>
{<br>
    ting::Ptr&lt;MyClass&gt; p1(new MyClass());<br>
<br>
    ting::Ptr&lt;MyClass&gt; p2;<br>
<br>
    //Here, the "p1" points to the object of class MyClass,<br>
    //and "p2" is invalid (does not point to any object).<br>
<br>
    //access some members of the object<br>
    p1-&gt;a = 213;<br>
    <br>
    //assign "p1" to "p2"<br>
    p2 = p1;<br>
<br>
    //Here, the "p2" points to the object of class MyClass,<br>
    //and "p1" is invalid (does not point to any object).<br>
    //I.e. the owning of the object is transferred to "p2".<br>
}<br>
//Here, the object will be deleted, because auto pointer<br>
//which owns the object went out of scope.<br>
<br>
</code></pre>



<br>
<h1>ting::Array usage</h1>

While <i>ting::Ptr</i> is a wrapper above <code>new</code>/<code>delete</code> operators, the <i>ting::Array</i> is a wrapper above <code>new[]</code>/<code>delete[]</code> operators. Its behavior is similar to <i>ting::Ptr</i>.<br>
<br>
Example:<br>
<pre><code>class MyClass{<br>
public:<br>
    int a;<br>
};<br>
<br>
{<br>
    ting::Array&lt;MyClass&gt; a1(10);//create array of 10 instances of MyClass<br>
<br>
    ting::Array&lt;MyClass&gt; a2; //create empty array<br>
<br>
    //access 4th object (numeration starts from 0)<br>
    a1[3].a = 30;<br>
<br>
    //Here, the "a1" is an array of 10 objects,<br>
    //while "a2" is invalid (empty array).<br>
    <br>
    //assign "a1" to "a2"<br>
    a2 = a1;<br>
<br>
    //Here, the "a2" is an array of 10 objects,<br>
    //while "a1" is invalid (empty array).<br>
}<br>
//Here, the memory occupied by array will be freed,<br>
//and objects will destroyed.<br>
</code></pre>



<br>
<h1>No "delete" concept</h1>

Ideally, one should use only auto pointers in their programs, and should not use traditional C pointers. And it will significantly reduce the program memory leak proneness.<br>
<br>
<b>ting</b> offers 3 kinds of automatic "pointers":<br>
<ul><li><b>ting::Ptr</b>
</li><li><b>ting::Array</b>
</li><li><b>ting::Ref</b></li></ul>

Proper use of these facilities in your program will reduce the memory leaks proneness significantly.<br>
<br>
<b>Rule of thumb</b><br>
The keywords<br>
<code>delete</code>
and<br>
<code>delete[]</code>
should not be present in your programs. If they are then you might be not using automatic pointers somewhere.