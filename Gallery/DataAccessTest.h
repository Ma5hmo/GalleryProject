#pragma once
#include "DatabaseAccess.h"

class DataAccessTest
{
public:
	DataAccessTest();
	~DataAccessTest();

	void createTables();
	void addUsers();

private:
	static const char* _dbFileName;
	DatabaseAccess _dba;
};

