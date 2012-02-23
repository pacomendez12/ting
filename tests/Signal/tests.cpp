#include "../../src/ting/debug.hpp"
#include "../../src/ting/Signal.hpp"
#include "../../src/ting/Ref.hpp"

#include "tests.hpp"




//Test function signal-slot connections
namespace FuncConnectionTest{

static unsigned a = 0;
static int b = 100;

static unsigned FuncDoSomethingWithA(){
	a = 34;
	return a;
}

//NOTE, the only accepted arguments are (3, 5002)
static unsigned FuncDoSomethingWithA1(int k, long b){
	ASSERT(k == 3)
	ASSERT(b == 5002)
	a = 344;
	return a;
}

//NOTE, the only accepted arguments are (3, 5002, 0)
static unsigned FuncDoSomethingWithA1(int k, long b, char* g){
	ASSERT(k == 3)
	ASSERT(b == 5002)
	ASSERT(g == 0)
	a = 3446;
	return a;
}

static int FuncDoSomethingWithB(){
	b = 41;
	return b;
}


void Run(){
	//test zero parameters signal
	{
		ting::Signal0 sig;
		ASSERT_ALWAYS(sig.NumConnections() == 0)

		sig.Connect(&FuncDoSomethingWithA);
		ASSERT_ALWAYS(sig.NumConnections() == 1)
		ASSERT_ALWAYS(sig.IsConnected(&FuncDoSomethingWithA))
		ASSERT_ALWAYS(!sig.IsConnected(&FuncDoSomethingWithB))

		ASSERT_ALWAYS(a == 0)
		sig.Emit();
		ASSERT_ALWAYS(a == 34)

		a = 0;

		sig.Connect(&FuncDoSomethingWithB);
		ASSERT_ALWAYS(sig.NumConnections() == 2)
		ASSERT_ALWAYS(sig.IsConnected(&FuncDoSomethingWithA))
		ASSERT_ALWAYS(sig.IsConnected(&FuncDoSomethingWithB))

		ASSERT_ALWAYS(a == 0)
		ASSERT_ALWAYS(b == 100)
		sig.Emit();
		ASSERT_ALWAYS(a == 34)
		ASSERT_ALWAYS(b == 41)

		ASSERT_ALWAYS(sig.NumConnections() == 2)
		sig.DisconnectAll();
		ASSERT_ALWAYS(sig.NumConnections() == 0)
		ASSERT_ALWAYS(!sig.IsConnected(&FuncDoSomethingWithA))
		ASSERT_ALWAYS(!sig.IsConnected(&FuncDoSomethingWithB))

		a = 0;
		b = 100;
		sig.Emit();
		ASSERT_ALWAYS(a == 0)
		ASSERT_ALWAYS(b == 100)
	}


	//Test multiple parameters signal
	{
		ting::Signal3<int, long, char*> sig;
		ASSERT_ALWAYS(sig.NumConnections() == 0)

		a = 0;
		b = 100;

		sig.Connect((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1);
		ASSERT_ALWAYS(sig.NumConnections() == 1)
		ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
		ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
		ASSERT_ALWAYS(!sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
		ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))

		sig.Emit(3, 5002, 0);
		ASSERT_ALWAYS(a == 3446)

		sig.DisconnectAll();
		ASSERT_ALWAYS(sig.NumConnections() == 0)
		ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
		ASSERT_ALWAYS(!sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
		ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
		ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))

		sig.Connect((unsigned(*)()) &FuncDoSomethingWithA);
		ASSERT_ALWAYS(sig.NumConnections() == 1)
		ASSERT_ALWAYS(sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
		ASSERT_ALWAYS(!sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
		ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
		ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))
		sig.Connect((int(*)()) &FuncDoSomethingWithB);
		ASSERT_ALWAYS(sig.NumConnections() == 2)
		ASSERT_ALWAYS(sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
		ASSERT_ALWAYS(sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
		ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
		ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))
		sig.Connect((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1);
		ASSERT_ALWAYS(sig.NumConnections() == 3)
		ASSERT_ALWAYS(sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
		ASSERT_ALWAYS(sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
		ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
		ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))
		sig.Connect((unsigned(*)(int, long)) &FuncDoSomethingWithA1);
		ASSERT_ALWAYS(sig.NumConnections() == 4)
		ASSERT_ALWAYS(sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
		ASSERT_ALWAYS(sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
		ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
		ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))

		sig.Emit(3, 5002, 0);
		ASSERT_ALWAYS(a == 344)
		ASSERT_ALWAYS(b == 41)

		//
		//
		//    test disconnection
		//
		//
		a = 0;
		b = 100;

		{
			bool res = sig.Disconnect(&FuncDoSomethingWithB);
			ASSERT_ALWAYS(res == true)
			ASSERT_ALWAYS(sig.NumConnections() == 3)
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
			ASSERT_ALWAYS(!sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))

			res = sig.Disconnect(&FuncDoSomethingWithB);
			ASSERT_ALWAYS(res == false)
			ASSERT_ALWAYS(sig.NumConnections() == 3)
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
			ASSERT_ALWAYS(!sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))
		}

		sig.Emit(3, 5002, 0);
		ASSERT_ALWAYS(a == 344)
		ASSERT_ALWAYS(b == 100)



		a = 0;
		b = 100;

		{
			bool res = sig.Disconnect((unsigned(*)(int, long)) &FuncDoSomethingWithA1);
			ASSERT_ALWAYS(res == true)
			ASSERT_ALWAYS(sig.NumConnections() == 2)
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
			ASSERT_ALWAYS(!sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
			ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))

			res = sig.Disconnect((unsigned(*)(int, long)) &FuncDoSomethingWithA1);
			ASSERT_ALWAYS(res == false)
			ASSERT_ALWAYS(sig.NumConnections() == 2)
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
			ASSERT_ALWAYS(!sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
			ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))
		}

		sig.Emit(3, 5002, 0);
		ASSERT_ALWAYS(a == 3446)
		ASSERT_ALWAYS(b == 100)



		a = 0;
		b = 100;

		{
			bool res = sig.Disconnect((unsigned(*)()) &FuncDoSomethingWithA);
			ASSERT_ALWAYS(res == true)
			ASSERT_ALWAYS(sig.NumConnections() == 1)
			ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
			ASSERT_ALWAYS(!sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
			ASSERT_ALWAYS(sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
			ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))

			res = sig.Disconnect((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1);
			ASSERT_ALWAYS(res == true)
			ASSERT_ALWAYS(sig.NumConnections() == 0)
			ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
			ASSERT_ALWAYS(!sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
			ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
			ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))

			res = sig.Disconnect((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1);
			ASSERT_ALWAYS(res == false)
			ASSERT_ALWAYS(sig.NumConnections() == 0)
			ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)()) &FuncDoSomethingWithA))
			ASSERT_ALWAYS(!sig.IsConnected((int(*)()) &FuncDoSomethingWithB))
			ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long, char*)) &FuncDoSomethingWithA1))
			ASSERT_ALWAYS(!sig.IsConnected((unsigned(*)(int, long)) &FuncDoSomethingWithA1))
		}

		sig.Emit(3, 5002, 0);
		ASSERT_ALWAYS(a == 0)
		ASSERT_ALWAYS(b == 100)
	}
}

}//~namespace














//Test method signal-slot connections
namespace MethodConnectionTest{

class TestClass{
public:
	int a;
	int b;

	TestClass() :
			a(0),
			b(100)
	{}

	void DoSomethingWithA(){
		this->a = 87;
	}

	//NOTE: the only acceptable argument is (444)
	void DoSomethingWithA(long f){
		ASSERT(f == 444)
		this->a = 874;
	}

	unsigned DoSomethingWithB(){
		this->b = 376;
		return 2341234;
	}

	//NOTE: the only acceptable arguments are (7, 0, 8)
	int DoSomethingWithB(unsigned m, char* g, int l){
		ASSERT(m == 7)
		ASSERT(g == 0)
		ASSERT(l == 8)
		this->b = 37642;
		return 2342345;
	}

	//NOTE: the only acceptable arguments are (7, 0)
	unsigned DoSomethingWithB1(unsigned m, char* g){
		ASSERT(m == 7)
		ASSERT(g == 0)
		this->b = 3742;
		return 77653;
	}
};



void Run(){
	TestClass tc;

	{
		ting::Signal3<unsigned, char*, int> sig;
		ASSERT_ALWAYS(sig.NumConnections() == 0)

		sig.Connect(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA);
		ASSERT_ALWAYS(sig.NumConnections() == 1)
		ASSERT_ALWAYS(sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
		ASSERT_ALWAYS(!sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
		ASSERT_ALWAYS(!sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
		ASSERT_ALWAYS(!sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))

		sig.Emit(7, 0, 8);
		ASSERT_ALWAYS(tc.a == 87)

		sig.DisconnectAll();
		ASSERT_ALWAYS(sig.NumConnections() == 0)
		ASSERT_ALWAYS(!sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
		ASSERT_ALWAYS(!sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
		ASSERT_ALWAYS(!sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
		ASSERT_ALWAYS(!sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))

		//
		tc.a = 0;
		tc.b = 100;

		sig.Connect(&tc, &TestClass::DoSomethingWithB1);
		ASSERT_ALWAYS(sig.NumConnections() == 1)
		ASSERT_ALWAYS(!sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
		ASSERT_ALWAYS(sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
		ASSERT_ALWAYS(!sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
		ASSERT_ALWAYS(!sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))

		sig.Emit(7, 0, 8);
		ASSERT_ALWAYS(tc.a == 0)
		ASSERT_ALWAYS(tc.b == 3742)

		sig.Connect(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB);
		sig.Connect(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA);
		ASSERT_ALWAYS(sig.NumConnections() == 3)
		ASSERT_ALWAYS(sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
		ASSERT_ALWAYS(sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
		ASSERT_ALWAYS(sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
		ASSERT_ALWAYS(!sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))

		sig.Emit(7, 0, 8);
		ASSERT_ALWAYS(tc.a == 87)
		ASSERT_INFO_ALWAYS(tc.b == 376, "tc.b = " << tc.b)

		sig.Connect(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB);
		ASSERT_ALWAYS(sig.NumConnections() == 4)
		ASSERT_ALWAYS(sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
		ASSERT_ALWAYS(sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
		ASSERT_ALWAYS(sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
		ASSERT_ALWAYS(sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))

		sig.Emit(7, 0, 8);
		ASSERT_ALWAYS(tc.a == 87)
		ASSERT_INFO_ALWAYS(tc.b == 37642, "tc.b = " << tc.b)


		//
		//  Test disconnection
		//

		tc.a = 0;
		tc.b = 100;

		{
			bool res = sig.Disconnect(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA);
			ASSERT_ALWAYS(res == true)
			ASSERT_ALWAYS(sig.NumConnections() == 3)
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
			ASSERT_ALWAYS(sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
			ASSERT_ALWAYS(sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
			ASSERT_ALWAYS(sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))

			res = sig.Disconnect(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA);
			ASSERT_ALWAYS(res == false)
			ASSERT_ALWAYS(sig.NumConnections() == 3)
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
			ASSERT_ALWAYS(sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
			ASSERT_ALWAYS(sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
			ASSERT_ALWAYS(sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))
		}

		sig.Emit(7, 0, 8);
		ASSERT_ALWAYS(tc.a == 0)
		ASSERT_INFO_ALWAYS(tc.b == 37642, "tc.b = " << tc.b)



		{
			bool res = sig.Disconnect(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB);
			ASSERT_ALWAYS(res == true)
			ASSERT_ALWAYS(sig.NumConnections() == 2)
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
			ASSERT_ALWAYS(sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
			ASSERT_ALWAYS(sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))

			res = sig.Disconnect(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB);
			ASSERT_ALWAYS(res == true)
			ASSERT_ALWAYS(sig.NumConnections() == 1)
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
			ASSERT_ALWAYS(sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))

			res = sig.Disconnect(&tc, &TestClass::DoSomethingWithB1);
			ASSERT_ALWAYS(res == true)
			ASSERT_ALWAYS(sig.NumConnections() == 0)
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
			ASSERT_ALWAYS(!sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))

			res = sig.Disconnect(&tc, &TestClass::DoSomethingWithB1);
			ASSERT_ALWAYS(res == false)
			ASSERT_ALWAYS(sig.NumConnections() == 0)
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (void(TestClass::*)()) &TestClass::DoSomethingWithA))
			ASSERT_ALWAYS(!sig.IsConnected(&tc, &TestClass::DoSomethingWithB1))
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (unsigned(TestClass::*)()) &TestClass::DoSomethingWithB))
			ASSERT_ALWAYS(!sig.IsConnected(&tc, (int(TestClass::*)(unsigned, char*, int)) &TestClass::DoSomethingWithB))
		}
	}
}

}//~namespace











//Test WeakRef signal-slot connections
namespace WeakRefConnectionTest{

class TestClass : public ting::RefCounted{
public:
	int a;

	TestClass() :
			a(0)
	{}

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}

	int DoSomething(int newA){
		this->a = newA;
		return this->a;
	}

	int DoSomething(){
		this->a = 10;
		return this->a;
	}
};



void Run(){
	{
		ting::Signal0 signal0;
		ting::Signal1<int> signal1;

		ting::Ref<TestClass> a = TestClass::New();

		ting::WeakRef<TestClass> wa(a);

		signal0.Connect(wa, (int (TestClass::*)()) &TestClass::DoSomething);
		ASSERT_ALWAYS(signal0.IsConnected(wa, (int (TestClass::*)()) &TestClass::DoSomething))

		signal1.Connect(wa, (int(TestClass::*)(int)) &TestClass::DoSomething);
		ASSERT_ALWAYS(signal1.IsConnected(wa, (int(TestClass::*)(int)) &TestClass::DoSomething))

		ASSERT_ALWAYS(a.IsValid())
		ASSERT_ALWAYS(a->a == 0)

		signal0.Emit();
		ASSERT_ALWAYS(a.IsValid())
		ASSERT_ALWAYS(a->a == 10)

		signal1.Emit(3);
		ASSERT_ALWAYS(a.IsValid())
		ASSERT_ALWAYS(a->a == 3)


		//after destroying last hard reference, the weak reference becomes invalid.
		a.Reset();

		//NOTE: at this point number of connections should be 1 because it has a weak reference connection,
		//but after emitting the signal this connection should be removed because weak reference become invalid.
		ASSERT_ALWAYS(signal0.NumConnections() == 1)
		signal0.Emit();
		ASSERT_ALWAYS(signal0.NumConnections() == 0)
		ASSERT_ALWAYS(a.IsNotValid())

		//NOTE: at this point number of connections should be 1 because it has a weak reference connection,
		//but after checking for "is connected" this connection should be removed because weak reference become invalid.
		ASSERT_ALWAYS(signal1.NumConnections() == 1)
		ASSERT_ALWAYS(!signal1.IsConnected(wa, (int(TestClass::*)(int)) &TestClass::DoSomething))
		ASSERT_ALWAYS(signal1.NumConnections() == 0)
		ASSERT_ALWAYS(a.IsNotValid())
	}

	//Test disconnection
	{
		ting::Signal3<int, long, char*> sig;

		ting::Ref<TestClass> tc = TestClass::New();
		ting::WeakRef<TestClass> wa(tc);


		sig.Connect(wa, (int(TestClass::*)(int)) &TestClass::DoSomething);
		ASSERT_ALWAYS(sig.IsConnected(wa, (int(TestClass::*)(int)) &TestClass::DoSomething))

		sig.Connect(wa, (int(TestClass::*)()) &TestClass::DoSomething);
		ASSERT_ALWAYS(sig.NumConnections() == 2)
		ASSERT_ALWAYS(sig.IsConnected(wa, (int(TestClass::*)(int)) &TestClass::DoSomething))
		ASSERT_ALWAYS(sig.IsConnected(wa, (int(TestClass::*)()) &TestClass::DoSomething))


		tc->a = 0;

		sig.Emit(43, 0, 0);
		ASSERT_ALWAYS(tc->a == 10)

		{
			bool res = sig.Disconnect(wa, (int(TestClass::*)()) &TestClass::DoSomething);
			ASSERT_ALWAYS(res == true)
			ASSERT_ALWAYS(sig.NumConnections() == 1)
			ASSERT_ALWAYS(sig.IsConnected(wa, (int(TestClass::*)(int)) &TestClass::DoSomething))
			ASSERT_ALWAYS(!sig.IsConnected(wa, (int(TestClass::*)()) &TestClass::DoSomething))

			res = sig.Disconnect(wa, (int(TestClass::*)()) &TestClass::DoSomething);
			ASSERT_ALWAYS(res == false)
			ASSERT_ALWAYS(sig.NumConnections() == 1)
			ASSERT_ALWAYS(sig.IsConnected(wa, (int(TestClass::*)(int)) &TestClass::DoSomething))
			ASSERT_ALWAYS(!sig.IsConnected(wa, (int(TestClass::*)()) &TestClass::DoSomething))
		}

		sig.Emit(43, 0, 0);
		ASSERT_ALWAYS(tc->a == 43)


		//kill the object
		{
			tc.Reset();
			ting::Ref<TestClass> hl(wa);
			ASSERT_ALWAYS(hl.IsNotValid());
		}

		{
			ASSERT_ALWAYS(sig.NumConnections() == 1)//There should be one connection with invalid weak reference
			bool res = sig.Disconnect(
					wa,
					(int(TestClass::*)(int)) &TestClass::DoSomething
				);
			ASSERT_ALWAYS(res == false) //false because weak ref become invalid and the connection can't be identified anymore.
			ASSERT_ALWAYS(sig.NumConnections() == 0)
			ASSERT_ALWAYS(!sig.IsConnected(wa, (int(TestClass::*)(int)) &TestClass::DoSomething))
			ASSERT_ALWAYS(!sig.IsConnected(wa, (int(TestClass::*)()) &TestClass::DoSomething))

			res = sig.Disconnect(
					wa,
					(int(TestClass::*)(int)) &TestClass::DoSomething
				);
			ASSERT_ALWAYS(res == false)
			ASSERT_ALWAYS(sig.NumConnections() == 0)
			ASSERT_ALWAYS(!sig.IsConnected(wa, (int(TestClass::*)(int)) &TestClass::DoSomething))
			ASSERT_ALWAYS(!sig.IsConnected(wa, (int(TestClass::*)()) &TestClass::DoSomething))
		}
	}

	{
		ting::Signal0 signal0;

		ting::Ref<TestClass> a;

		{
			a = TestClass::New();

			signal0.Connect(
					a.GetWeakRef(),
					(int (TestClass::*)()) &TestClass::DoSomething
				);
			ASSERT_ALWAYS(signal0.NumConnections() == 1)
			ASSERT_ALWAYS(signal0.IsConnected(
					a.GetWeakRef(),
					(int (TestClass::*)()) &TestClass::DoSomething
				))

			signal0.Connect(
					a.GetWeakRef(),
					(int (TestClass::*)()) &TestClass::DoSomething
				);
			ASSERT_ALWAYS(signal0.NumConnections() == 2)
			ASSERT_ALWAYS(signal0.IsConnected(
					a.GetWeakRef(),
					(int (TestClass::*)()) &TestClass::DoSomething
				))

			a.Reset();
			//Now we have 2 dead connections (weak refs are invalid)
			ASSERT_ALWAYS(signal0.NumConnections() == 2)

			a = TestClass::New();
			//this connect should remove dead connections
			signal0.Connect(
					a.GetWeakRef(),
					(int (TestClass::*)()) &TestClass::DoSomething
				);
			//should be only one alive connection
			ASSERT_ALWAYS(signal0.NumConnections() == 1)

			ASSERT_ALWAYS(signal0.IsConnected(
					a.GetWeakRef(),
					(int (TestClass::*)()) &TestClass::DoSomething
				))
		}
	}
}

}//~namespace







namespace MixedConnectionTest{

class TestRefClass : public ting::RefCounted{
public:
	int a;

	void DoSomethingWithA(){
		this->a = 278;
	}

	static inline ting::Ref<TestRefClass> New(){
		return ting::Ref<TestRefClass>(new TestRefClass());
	}
};



class TestClass{
public:
	int a;

	void DoSomethingWithA(){
		this->a = 563;
	}
};



static int a;

static void DoSomethingWithA(){
	a = 852;
}



void Run(){
	ting::Ref<TestRefClass> rc = TestRefClass::New();
	TestClass tc;

	ting::Signal2<long, unsigned> sig;
	ASSERT_ALWAYS(sig.NumConnections() == 0)

	sig.Connect(&DoSomethingWithA);
	ASSERT_ALWAYS(sig.NumConnections() == 1)
	ASSERT_ALWAYS(sig.IsConnected(&DoSomethingWithA))
	ASSERT_ALWAYS(!sig.IsConnected(&tc, &TestClass::DoSomethingWithA))
	ASSERT_ALWAYS(!sig.IsConnected(ting::WeakRef<TestRefClass>(rc), &TestRefClass::DoSomethingWithA))

	a = 0;
	sig.Emit(0, 0);
	ASSERT_ALWAYS(a == 852)

	sig.Connect(&tc, &TestClass::DoSomethingWithA);
	ASSERT_ALWAYS(sig.NumConnections() == 2)
	ASSERT_ALWAYS(sig.IsConnected(&DoSomethingWithA))
	ASSERT_ALWAYS(sig.IsConnected(&tc, &TestClass::DoSomethingWithA))
	ASSERT_ALWAYS(!sig.IsConnected(ting::WeakRef<TestRefClass>(rc), &TestRefClass::DoSomethingWithA))

	a = 0;
	tc.a = 0;
	sig.Emit(23432, 2653);
	ASSERT_ALWAYS(a == 852)
	ASSERT_ALWAYS(tc.a == 563)

	{
		ting::WeakRef<TestRefClass> wr(rc);
		sig.Connect(wr, &TestRefClass::DoSomethingWithA);
		ASSERT_ALWAYS(sig.NumConnections() == 3)
		ASSERT_ALWAYS(sig.IsConnected(&DoSomethingWithA))
		ASSERT_ALWAYS(sig.IsConnected(&tc, &TestClass::DoSomethingWithA))
		ASSERT_ALWAYS(sig.IsConnected(wr, &TestRefClass::DoSomethingWithA))
	}

	a = 0;
	tc.a = 0;
	rc->a = 0;
	sig.Emit(27845, 3746);
	ASSERT_ALWAYS(a == 852)
	ASSERT_ALWAYS(tc.a == 563)
	ASSERT_ALWAYS(rc->a == 278)

	ASSERT_ALWAYS(sig.NumConnections() == 3)
	rc.Reset();
	sig.Emit(45, 25345);
	ASSERT_ALWAYS(sig.NumConnections() == 2)
	ASSERT_ALWAYS(sig.IsConnected(&DoSomethingWithA))
	ASSERT_ALWAYS(sig.IsConnected(&tc, &TestClass::DoSomethingWithA))
	ASSERT_ALWAYS(!sig.IsConnected(ting::WeakRef<TestRefClass>(rc), &TestRefClass::DoSomethingWithA))

	sig.DisconnectAll();
	ASSERT_ALWAYS(sig.NumConnections() == 0)
}

}//~namespace
