#pragma once
#include "DatabaseAccess.h"

class DataAccessTest
{
public:
	DataAccessTest();
	~DataAccessTest();

	void createTables();
	void addRows();

private:
	static const char* _dbFileName;
	DatabaseAccess _dba;
};

