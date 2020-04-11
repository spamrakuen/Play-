#pragma once

#include "TestVm.h"

#define TEST_VERIFY(a) \
	if(!(a))           \
	{                  \
		int* p = 0;    \
		(*p) = 0;      \
	}

class CTest
{
public:
	virtual ~CTest() = default;
	virtual void Execute(CTestVm&) = 0;

	enum Float
	{
		_0 = 0000000000,
		_1 = 0x3F800000,
		_2 = 0x40000000,
		_4 = 0x40800000,
		_8 = 0x41000000,
		_64 = 0x42800000,
		_256 = 0x43800000,
	};
};
