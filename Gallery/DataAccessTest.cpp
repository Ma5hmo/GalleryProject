#include "DataAccessTest.h"
#include <cstdio>

const char* DataAccessTest::_dbFileName = "testDB.sqlite";

DataAccessTest::DataAccessTest()
{
	std::remove(_dbFileName);
	_dba = DatabaseAccess(_dbFileName);
}

DataAccessTest::~DataAccessTest()
{
	_dba.close();
}

void DataAccessTest::createTables()
{
	std::cout << "Testing table creation..." << std::endl;
	try
	{
		_dba.open();
		std::cout << "SUCCESS!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "FAILED! Error - " << e.what() << std::endl;
	}
}
