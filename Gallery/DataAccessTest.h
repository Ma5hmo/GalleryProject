#pragma once
#include "DatabaseAccess.h"

class DataAccessTest
{
public:
	DataAccessTest();
	~DataAccessTest();

	void createTables();
	void addRows();
	void updateRows();
	void removeRows();

private:
	static const char* _dbFileName;
	DatabaseAccess _dba;
};

