#pragma once

#include "../../src/ting/Ref.hpp"



class TestClass : public ting::RefCounted{
public:
	int a;

	bool* destroyed;

	TestClass(bool* destroyed) :
			a(32),
			destroyed(ASS(destroyed))
	{
//		TRACE(<< "TestClass(): constructed, this->a = " << this->a << std::endl)
	}
	~TestClass()noexcept{
//		TRACE(<< "~TestClass(): destructed" << std::endl)
		*this->destroyed = true;
	}
};
