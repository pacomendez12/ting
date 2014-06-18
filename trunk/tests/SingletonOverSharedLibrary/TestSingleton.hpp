#pragma once

#include "../../src/ting/Singleton.hpp"



class TestSingleton : public ting::Singleton<TestSingleton>{
public:
	int a;

	TestSingleton() :
			a(32)
	{
//		TRACE(<< "TestSingleton(): constructed, this->a = " << this->a << std::endl)
	}
	~TestSingleton()throw(){
//		TRACE(<< "~TestSingleton(): destructed" << std::endl)
	}
};
