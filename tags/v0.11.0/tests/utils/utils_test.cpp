#include "../../src/ting/debug.hpp"
#include "../../src/ting/types.hpp"
#include "../../src/ting/utils.hpp"
#include "../../src/ting/Buffer.hpp"



using namespace ting;



static void TestSerialization(){
	//16 bit
	for(u32 i = 0; i <= u16(-1); ++i){
		STATIC_ASSERT(sizeof(u16) == 2)
		StaticBuffer<u8, sizeof(u16)> buf;
		ting::Serialize16LE(u16(i), buf.Begin());

		ASSERT_ALWAYS(buf[0] == u8(i & 0xff))
		ASSERT_ALWAYS(buf[1] == u8((i >> 8) & 0xff))

		u16 res = ting::Deserialize16LE(buf.Begin());
		ASSERT_ALWAYS(res == u16(i))
//		TRACE(<< "TestSerialization(): i16 = " << i << std::endl)
	}

	//32 bit
	for(u64 i = 0; i <= u32(-1); i += 1317){//increment by 1317, because if increment by 1 it takes too long to run the test
		STATIC_ASSERT(sizeof(u32) == 4)
		StaticBuffer<u8, sizeof(u32)> buf;
		ting::Serialize32LE(u32(i), buf.Begin());

		ASSERT_ALWAYS(buf[0] == u8(i & 0xff))
		ASSERT_ALWAYS(buf[1] == u8((i >> 8) & 0xff))
		ASSERT_ALWAYS(buf[2] == u8((i >> 16) & 0xff))
		ASSERT_ALWAYS(buf[3] == u8((i >> 24) & 0xff))

		u32 res = ting::Deserialize32LE(buf.Begin());
		ASSERT_ALWAYS(res == u32(i))
//		TRACE(<< "TestSerialization(): i32 = " << i << std::endl)
	}
}



int main(int argc, char *argv[]){
//	TRACE(<< "utils test" << std::endl)

	TestSerialization();

	TRACE_ALWAYS(<< "[PASSED]: utils test" << std::endl)

	return 0;
}
