/* The MIT License:

Copyright (c) 2008-2010 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// ting 0.4.2
// Homepage: http://code.google.com/p/ting

/**
 * @file Ptr.hpp
 * @author Ivan Gagis <igagis@gmail.com>
 * @brief Pointer wrapper.
 */

#pragma once

#include "debug.hpp"

//#define M_ENABLE_PTR_PRINT
#ifdef M_ENABLE_PTR_PRINT
#define M_PTR_PRINT(x) TRACE(x)
#else
#define M_PTR_PRINT(x)
#endif

namespace ting{


/**
 * @brief Auto-pointer template class.
 * Auto-pointer class is a wrapper above ordinary pointer.
 * It holds a pointer to an object and it will 'delete'
 * that object when pointer goes out of scope.
 */
template <class T> class Ptr{
	void* p;
public:
	explicit inline Ptr(T* ptr = 0) :
			p(ptr)
	{}

	/**
	 * @brief Copy constructor.
	 * Creates a copy of 'ptr' and invalidates it.
	 * This means that if creating Ptr object like this:
	 *     Ptr<SomeClass> a(new SomeClass());//create pointer 'a'
	 *     Ptr<SomeClass> b(a);//create pointer 'b' using copy constructor
	 * then 'a' will become invalid while 'b' will hold pointer to the object
	 * of class 'SomeClass' which 'a' was holding before.
	 * I.e. when using copy constructor, no memory allocation occurs,
	 * object kept by 'a' is moved to 'b' and 'a' is invalidated.
	 * @param ptr - pointer to copy.
	 */
	//const copy constructor
	inline Ptr(const Ptr& ptr){
		M_PTR_PRINT(<< "Ptr::Ptr(copy): invoked, ptr.p = " << (ptr.p) << std::endl)
		this->p = ptr.p;
		const_cast<Ptr&>(ptr).p = 0;
	}

	inline ~Ptr(){
		this->Destroy();
	}

	inline T* operator->(){
		ASSERT_INFO(this->p, "Ptr::operator->(): this->p is zero")
		return static_cast<T*>(p);
	}

	inline const T* operator->()const{
		ASSERT_INFO(this->p, "const Ptr::operator->(): this->p is zero")
		return static_cast<T*>(this->p);
	}

	inline T& operator*(){
		ASSERT_INFO(this->p, "Ptr::operator*(): this->p is zero")
		return *(this->operator->());
	}

	inline const T& operator*()const{
		ASSERT_INFO(this->p, "const Ptr::operator*(): this->p is zero")
		return *(this->operator->());
	}

	/**
	 * @brief Assignment operator.
	 * This operator works the same way as copy constructor does.
	 * That is, if assignng like this:
	 *     Ptr<SomeClass> b(new SomeClass()), a(new SomeClass());
	 *     b = a;
	 * then 'a' will become invalid and 'b' will hold the object owned by 'a' before.
	 * Note, that object owned by 'b' prior to assignment is deleted.
	 * Thus, no memory leak occurs.
	 * @param ptr - pointer to assign from.
	 */
	inline Ptr& operator=(const Ptr& ptr){
		M_PTR_PRINT(<< "Ptr::operator=(Ptr&): enter, this->p = " << (this->p) << std::endl)
		this->Destroy();
		this->p = ptr.p;
		const_cast<Ptr&>(ptr).p = 0;
		M_PTR_PRINT(<< "Ptr::operator=(Ptr&): exit, this->p = " << (this->p) << std::endl)
		return (*this);
	}

	inline bool operator==(const T* ptr)const{
		return this->p == ptr;
	}

	inline bool operator!=(const T* ptr)const{
		return !( *this == ptr );
	}

	inline bool operator!()const{
		return this->IsNotValid();
	}



	//Safe conversion to bool type.
	//Because if using simple "operator bool()" it may result in chained automatic
	//conversion to undesired types such as int.
	typedef void (Ptr::*unspecified_bool_type)();
	inline operator unspecified_bool_type() const{
		return this->IsValid() ? &Ptr::Reset : 0; //Ptr::Reset is taken just because it has matching signature
	}

//	inline operator bool(){
//		return this->IsValid();
//	}



	/**
	 * @brief Extract pointer to object invalidating the Ptr.
	 * Extract the pointer to object from this Ptr instance and invalidate
	 * the Ptr instance. After that, when this Ptr instance goes out of scope
	 * the object will not be deleted because Ptr instance is already invalid
	 * at this point.
	 * @return pointer to object previously owned by that Ptr instance.
	 */
	inline T* Extract(){
		T* pp = static_cast<T*>(this->p);
		this->p = 0;
		return pp;
	}

	/**
	 * @brief reset pointer, destroying object it point to.
	 * This will destroy the object this pointer points to if any.
	 * After that the pointer becomes invalid.
	 */
	inline void Reset(){
		this->Destroy();
		this->p = 0;
	}

	/**
	 * @brief tells if the pointer is valid or not.
	 * @return true if pointer is valid and holding some object.
	 * @return false otherwise.
	 */
	inline bool IsValid()const{
		return this->p != 0;
	}

	/**
	 * @brief tells if the pointer is valid or not.
	 * @return false if object is valid.
	 * @return true otherwise.
	 */
	inline bool IsNotValid()const{
		return !this->IsValid();
	}

	/**
	 * @brief Static cast.
	 * NOTE: use this method very carefully!!! It returns ordinary pointer
	 * to the object while the object itself is still owned by Ptr.
	 * Do not create other Ptr instances using that returned value!!! As it
	 * will cause double 'delete' when both Ptr instances go out of scope.
	 * @return pointer to casted class.
	 */
	template <class TS> inline TS* StaticCast(){
		return static_cast<TS*>(this->operator->());
	}

	//for automatic type downcast
	template <typename TBase> inline operator Ptr<TBase>(){
		M_PTR_PRINT(<< "Ptr::downcast(): invoked, p = " << (this->p) << std::endl)
		return Ptr<TBase>(this->Extract());
	}

private:
	inline void Destroy(){
		M_PTR_PRINT(<< "Ptr::~Ptr(): delete invoked, this->p = " << this->p << std::endl)
		delete static_cast<T*>(this->p);
	}

	inline void* operator new(size_t){
		ASSERT(false)//forbidden
		return reinterpret_cast<void*>(0);
	}

	inline void operator delete(void*){
		ASSERT(false)//forbidden
	}
};

}//~namespace ting
