/* The MIT License:

Copyright (c) 2008-2013 Ivan Gagis <igagis@gmail.com>

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

// Home page: http://ting.googlecode.com



/**
 * @file Exc.hpp
 * @author Ivan Gagis <igagis@gmail.com>
 * @brief Basic Exception class.
 */

#pragma once

#include <string.h>
#include <string>
#include <exception>

#include "util.hpp"

namespace ting{

/**
 * @brief Basic exception class
 */
class Exc : public std::exception{
	std::string message;
public:
	/**
	 * @brief Constructor.
     * @param message - human friendly error description.
     */
	inline Exc(const std::string& message = std::string()) :
			message(message)
	{}

	virtual ~Exc()NOEXCEPT{}

	/**
	 * @brief Returns a pointer to exception message.
	 * @return a pointer to objects internal memory buffer holding
	 *         the exception message null-terminated string.
	 *         Note, that after the exception object is destroyed
	 *         the pointer returned by this method become invalid.
	 */
	const char *What()const NOEXCEPT{
		return this->what();
	}



private:
	//override from std::exception
	const char *what()const NOEXCEPT{
		return this->message.c_str();
	}
};

}//~namespace ting
