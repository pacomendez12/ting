# Introduction #

**ting** provides facilities for creating reference counted objects and use reference counting pointers to these objects. This allows free the object memory automatically whenever no active references to that object left.

Strong references and weak references are supported.

Reference counted object may be referenced from different threads. _ting::RefCounted_ referencing mechanism is thread safe in the sense that if two threads hold strong reference to the object and in both of them the strong reference cease to exist, then the object will not be deleted twice, but only once, as required to avoid application crash.

Whenever there is a strong reference to the object, one can be assured that the object will exist and will not be deleted. The object is deleted when there are no strong references left.


<br>
<h1>Header file</h1>

In order to use the ting::Ref one needs to include the file:<br>
<pre><code>#include &lt;ting/Ref.hpp&gt;<br>
</code></pre>



<br>
<h1>Creating reference counted class</h1>

To create a class for your reference counted object just derive from <i>ting::RefCounted</i>, as follows:<br>
<pre><code>class MyClass : public ting::RefCounted{<br>
protected:<br>
    MyClass(int valueA, int valueB) :<br>
            a(valueA),<br>
            b(valueB)<br>
    {}<br>
public:<br>
    //Need to define destructor to indicate that it throws no exceptions.<br>
    ~MyClass()throw(){}<br>
<br>
    int a;<br>
    int b;<br>
<br>
    static ting::Ref&lt;MyClass&gt; New(int valueA, int valueB){<br>
        return ting::Ref&lt;MyClass&gt;(<br>
                new MyClass(valueA, valueB)<br>
            );<br>
    }<br>
};<br>
</code></pre>
Note, that constructor of the class is intentionally made protected and a static New() method introduced to construct the object. This is not a mandatory technique but it is a good practice, and using this with <i>ting::RefCounted</i> objects is encouraged. As of current version (ting-0.4) the operator <b>new</b> of <i>ting::RefCounted</i> class is public, but it is possible that in later versions of ting it will be made protected, so it will only be possible to construct the object from within its own method, e.g. New(). Also, using this practice will help you assure that the object of your class will only be accessed through <i>ting::Ref</i> references, and not via traditional C pointers, whose usage with <i>ting::RefCounted</i>, if not forbidden, then, at least, strongly <i>not</i> recommended.<br>
<br>
<br>
<br>
<br>
<h1>Referencing objects with <i>ting::Ref</i></h1>

<i>ting::Ref</i> is a template class representing a strong reference to reference counted object (derived from <i>ting::RefCounted</i>). Note, that it is not allowed to create references using <b>new</b> operator (the <b>new</b> operator is private anyway).<br>
<br>
The usage is quite simple:<br>
<br>
<pre><code>//create first object<br>
ting::Ref&lt;MyClass&gt; globalMC = MyClass::New(0, 1);;<br>
<br>
{<br>
    //create second object<br>
    ting::Ref&lt;MyClass&gt; mc = MyClass::New(13, 14);<br>
<br>
    mc-&gt;a = 15;<br>
    int k = mc-&gt;b;<br>
<br>
    globalMC = mc;<br>
    //At this point, the first object will be deleted.<br>
    //And "globalRC" reference is pointing to second object as "mc" does.<br>
}<br>
//At this point the "mc" reference is destroyed as it went out of scope.<br>
//But "globalMC" still references to the second object.<br>
<br>
glomalMC.Reset();//Reset this reference, it will no longer point to any object.<br>
//At this point, the second object will be deleted, as the last strong reference<br>
//to it was destroyed, or reset.<br>
</code></pre>



<br>
<h1>Referencing objects with <i>ting::WeakRef</i></h1>

<i>ting::WeakRef</i> is a template class representing a weak reference. Weak reference does not guarantee that the object is still alive. Actually, if there are several strong references to the object and there are several weak references to the same object, then, after all strong references go out of scope the object will be destroyed immediately, despite the fact that some weak references still exist.<br>
<br>
Note, that it is not allowed to access the object through weak reference directly. In order to access the object, one needs to construct a strong reference from the weak reference and then check that the constructed strong reference is valid. If the constructed strong reference is not valid, then the object was deleted before because all strong references to it have ceased to exist.<br>
<br>
The usage is as follows:<br>
<pre><code>//create object<br>
ting::Ref&lt;MyClass&gt; mc = MyClass::New(13, 14);<br>
<br>
//create weak reference to the object<br>
ting::WeakRef&lt;MyClass&gt; wmc = mc;<br>
<br>
//access some object member using the weak reference<br>
if(ting::Ref&lt;MyClass&gt; hmc = wmc){<br>
    hmc-&gt;a = 35;<br>
    int k = hmc-&gt;b;<br>
    //...<br>
}<br>
<br>
//destroy last strong reference to the object<br>
mc.Reset();//the object will be deleted<br>
<br>
//try accessing some object member using the weak reference<br>
if(ting::Ref&lt;MyClass&gt; hmc = wmc){<br>
   //Will never get there as the object is destroyed and<br>
   //weak reference in not valid anymore<br>
}<br>
</code></pre>



<br>
<h1>Memory leaks</h1>

The purpose of the reference counting mechanism is to avoid memory leaks, but, nevertheless, memory leaks are still possible. The scenario of memory leaking when using reference counting is called a cycling references.<br>
<br>
If there are two objects which hold strong references to each other then these two objects will never be destroyed, even if all strong references to them from "outside" world have went out of scope. Because there are still at least 1 strong reference to each of the object exists (reference being held by another object).<br>
<br>
To avoid such situations, one needs to do a proper design of his/her classes and use weak reference in one of such objects and strong reference in another one. This is the <i>main</i> reason why weak references are introduced.<br>
<br>
<br>
<br>
<br>
<h1>Creating strong and weak references from pointer</h1>

It is possible to construct the Ref and WeakRef objects passing the ordinary pointer to the RefCounted object as argument to corresponding constructor. In general, it is <i>NOT RECOMMENDED</i> to do this, except for the following two cases:<br>
<ol><li>creating the first strong reference in static New() method.<br>
</li><li>creating strong or weak reference using "this" pointer inside of the RefCounted object methods.</li></ol>

NOTE: The 2nd case has exception: Creating <i>strong</i> references using "this" pointer from within the constructor is NOT recommended, if you do so, be prepared to very probable program crash. Creating weak references using "this" pointer from within the constructor IS ok.<br>
<br>
<pre><code>class TestClass : virtual public ting::RefCounted{<br>
public:<br>
<br>
    TestClass(){<br>
        ting::Ref&lt;TestClass&gt; sr(this); //!!! do not do this, will lead to crash later!<br>
        ting::WeakRef&lt;TestClass&gt; wr(this); // OK!<br>
    }<br>
<br>
    ~TestClass()throw(){}<br>
<br>
    void Func(){<br>
        ting::Ref&lt;TestClass&gt; sr(this); // OK!<br>
        ting::WeakRef&lt;TestClass&gt; wr(this); // OK!<br>
    }<br>
<br>
    static ting::Ref&lt;TestClass&gt; New(){<br>
        return ting::Ref&lt;TestClass&gt;(new TestClass()); //OK!<br>
    }<br>
};<br>
</code></pre>

NOTE: note the usage of virtual inheritance in this example when deriving from RefCounted, it is allowed and in some cases is  useful. In this example the virtual inheritance is used only for demonstration purposes, in fact it is not required here.<br>
<br>
<br>
<br>
<br>
<h1>Constant references and references to constant objects</h1>

Constant references:<br>
<pre><code>const ting::Ref&lt;TestClass&gt; sr;<br>
const ting::WeakRef&lt;TestClass&gt; wr;<br>
</code></pre>
you can't call non-const methods of these references, but you can do it for the object they are referring to.<br>
<br>
References to constant objects:<br>
<pre><code>ting::Ref&lt;const TestClass&gt; sr;<br>
ting::WeakRef&lt;const TestClass&gt; wr;<br>
</code></pre>
You cannot call non-const methods on the object referenced by these references.<br>
You can create reference to a const object out of the reference to non-const object (using copy constructor or assignment operator).