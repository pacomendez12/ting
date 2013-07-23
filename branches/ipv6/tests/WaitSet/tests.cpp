#include <vector>

#include "../../src/ting/debug.hpp"
#include "../../src/ting/WaitSet.hpp"
#include "../../src/ting/mt/MsgThread.hpp"

#include "tests.hpp"



namespace test_message_queue_as_waitable{

class TestThread : public ting::mt::MsgThread{
public:
	//override
	void Run(){
		ting::WaitSet ws(1);

		ws.Add(this->queue, ting::Waitable::READ);

		unsigned res = ws.WaitWithTimeout(3000, 0);

		ASSERT_ALWAYS(res == 1)

		ws.Remove(this->queue);
	}
};

void Run(){
	TestThread t;

	t.Start();

	ting::mt::Thread::Sleep(1000);

	t.PushNopMessage();

	t.Join();
}
}//~namespace



namespace test_general{
void Run(){
	ting::WaitSet ws(4);

	ting::mt::Queue q1, q2;

	ws.Add(q1, ting::Waitable::READ);
	ws.Add(q2, ting::Waitable::READ);

	ting::StaticBuffer<ting::Waitable*, 4> buf;



	//test Wait() with zero timeout, no objects should trigger, so, expecting return value of 0.
	ASSERT_ALWAYS(ws.WaitWithTimeout(0) == 0)
	ASSERT_ALWAYS(ws.WaitWithTimeout(0, &buf) == 0)



	//test Wait() with non-zero timeout, no objects should trigger, so, expecting return value of 0.
	ASSERT_ALWAYS(ws.WaitWithTimeout(100) == 0)
	ASSERT_ALWAYS(ws.WaitWithTimeout(100, &buf) == 0)



	//test Wait with 1 triggered object
	q1.PushMessage(ting::Ptr<ting::mt::Message>(new ting::mt::NopMessage()));
	ASSERT_ALWAYS(ws.Wait() == 1)
	ASSERT_ALWAYS(ws.Wait(&buf) == 1)
	ASSERT_ALWAYS(buf[0] == &q1)

	ASSERT_ALWAYS(ws.WaitWithTimeout(100) == 1)
	ASSERT_ALWAYS(ws.WaitWithTimeout(100, &buf) == 1)
	ASSERT_ALWAYS(buf[0] == &q1)
	ASSERT_ALWAYS(!q2.CanRead())

	//check that no objects trigger after reading from queue
	q1.GetMsg();//should not block since one message was pushed before
	ASSERT_ALWAYS(ws.WaitWithTimeout(100) == 0)
	ASSERT_ALWAYS(ws.WaitWithTimeout(100, &buf) == 0)



	//test Wait with 2 triggered objects
	q1.PushMessage(ting::Ptr<ting::mt::Message>(new ting::mt::NopMessage()));
	q2.PushMessage(ting::Ptr<ting::mt::Message>(new ting::mt::NopMessage()));
	ASSERT_ALWAYS(ws.Wait() == 2)
	ASSERT_ALWAYS(ws.Wait(&buf) == 2)
	ASSERT_ALWAYS((buf[0] == &q1 && buf[1] == &q2) || (buf[0] == &q2 && buf[1] == &q1))

	ASSERT_ALWAYS(ws.WaitWithTimeout(100) == 2)
	ASSERT_ALWAYS(ws.WaitWithTimeout(100, &buf) == 2)
	ASSERT_ALWAYS((buf[0] == &q1 && buf[1] == &q2) || (buf[0] == &q2 && buf[1] == &q1))

	//check that no objects trigger after reading from queue
	q1.GetMsg();//should not block since one message was pushed before
	ASSERT_ALWAYS(ws.WaitWithTimeout(100) == 1)
	ASSERT_ALWAYS(ws.WaitWithTimeout(100, &buf) == 1)
	ASSERT_ALWAYS(buf[0] == &q2)

	q2.GetMsg();//should not block since one message was pushed before
	ASSERT_ALWAYS(ws.WaitWithTimeout(100) == 0)
	ASSERT_ALWAYS(ws.WaitWithTimeout(100, &buf) == 0)



	ws.Remove(q1);
	ws.Remove(q2);
}
}//~namespace
