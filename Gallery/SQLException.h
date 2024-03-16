#pragma once
#include "MyException.h"

class SQLException : public MyException
{
public:
	SQLException(const std::string& message) : MyException("SQL Error: " + message) {}
};
