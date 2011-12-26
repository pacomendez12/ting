/* The MIT License:

Copyright (c) 2008-2011 Ivan Gagis <igagis@gmail.com>

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

// Homepage: http://ting.googlecode.com



/**
 * @file types.hpp
 * @author Ivan Gagis <igagis@gmail.com>
 * @brief General types definitions.
 */

#pragma once

#include "config.hpp"
#include "debug.hpp"



//disable warning about ignored volatile in type cast when T is some volatile type
#if M_COMPILER == M_COMPILER_MSVC
#pragma warning(push) //push warnings state
#pragma warning( disable : 4197)
#endif



namespace ting{



/**
 * @brief Maximal value of unsigned integer type.
 */
const unsigned DMaxUInt = unsigned(-1);

/**
 * @brief Maximal value of integer type.
 */
const int DMaxInt = int(DMaxUInt >> 1);

/**
 * @brief Minimal value of integer type.
 */
const int DMinInt = ~DMaxInt;



/**
 * @brief Unsigned 8 bit type.
 */
typedef unsigned char u8;

/**
 * @brief Signed 8 bit type.
 */
typedef signed char s8;

/**
 * @brief Unsigned 16 bit type.
 */
typedef unsigned short int u16;

/**
 * @brief Signed 16 bit type.
 */
typedef signed short int s16;

/**
 * @brief Unsigned 32 bit type.
 */
typedef unsigned int u32;

/**
 * @brief Signed 32 bit type.
 */
typedef signed int s32;

/**
 * @brief Unsigned 64 bit type.
 */
typedef unsigned long long int u64;

/**
 * @brief Signed 64 bit type.
 */
typedef long long int s64;


#ifndef M_DOXYGEN_DONT_EXTRACT //for doxygen

STATIC_ASSERT(u8(-1) == 0xff)//assert that byte consists of exactly 8 bits, e.g. some systems have 10 bits per byte!!!
STATIC_ASSERT(sizeof(u8) == 1)
STATIC_ASSERT(sizeof(s8) == 1)
STATIC_ASSERT(sizeof(u16) == 2)
STATIC_ASSERT(sizeof(s16) == 2)
STATIC_ASSERT(sizeof(u32) == 4)
STATIC_ASSERT(sizeof(s32) == 4)
STATIC_ASSERT(sizeof(u64) == 8)
STATIC_ASSERT(u64(-1) == 0xffffffffffffffffLL)
STATIC_ASSERT(sizeof(s64) == 8)

#endif //~M_DOXYGEN_DONT_EXTRACT //for doxygen



/**
 * @brief Thin wrapper above any C++ built-in type allowing initialization from int.
 * Thin wrapper above any C++ built-in type which allows initialization from C++ int type.
 * This wrapper allows initialize the variable to some int value right
 * in place of declaration.
 * This is useful when declaring class members, so you can
 * indicate to which value the variable should be initialized without using
 * constructor initialization list or assigning the value in the constructor body.
 * Auto-conversions to/from the original type are supported.
 * Note, that in order to wrap const value to Inited one has to use const qualifier to a
 * template argument, not to the whole Inited class. Const qualifier to the whole Inited class
 * has no any effect.
 *
 * Typical usage:
 * @code
 * #include <ting/types.hpp>
 *
 * //...
 *
 * class SampleClass{
 * public:
 *     ting::Inited<float, 0> x; //initialized to 0 upon object construction
 *     ting::Inited<float, -10> y; //initialized to -10 upon object construction
 *     ting::Inited<const float, 10> width; //constant value initialized to 10 upon object construction
 *     ting::Inited<float, 20> height; //initialized to 20 upon object construction
 *     ting::Inited<bool, false> visible; //bool variable initialized to false
 *     ting::Inited<volatile bool, true> someFlag; //volatile bool variable initialized to true
 *     const ting::Inited<int, 20> nonConstValue; //non-constant value initialized to 20. const outside of template eargument has no any effect.
 * }
 * @endcode
 */
template <class T, int V = 0> class Inited{
	T value;
	
public:


	inline Inited() :
			value(T(V))
	{}



	inline Inited(T value) :
			value(value)
	{}
	
	//This template operator= is needed to assign Inited to another Inited
	//which had different initial value. Note, that the default assignment
	//operator is still generated by the compiler,
	// i.e. this template does not work for two Inited instances of the exact same T and V.
	template <class T2, int K> inline Inited<T, V>& operator=(const Inited<T2, K>& inited)const{
		const_cast<T&>(this->value) = inited;
		return *const_cast<Inited*>(this);
	}
	
	inline Inited& operator=(const Inited& inited)const{
		return this->operator=<T, V>(inited);
	}
	
	inline operator T&()const{
		return const_cast<T&>(this->value);
	}

	inline T operator->(){
		return this->value;
	}
	
	inline const T operator->()const{
		return this->value;
	}

	inline T* operator&()const{
		return const_cast<T*>(&this->value);
	}
};



}//~namespace ting



//restore MSVC compiler warnings state
#if M_COMPILER == M_COMPILER_MSVC
#pragma warning(pop) //pop warnings state
#endif
