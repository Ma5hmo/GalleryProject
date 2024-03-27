#pragma once
#include "DatabaseAccess.h"

class DataAccessTest
{
public:
	DataAccessTest();
	~DataAccessTest();

	void runTests();

	void createTables();
	void addRows();
	void updateRows();
	void removeRows();

private:
	static constexpr char* _dbFileName = "testDB.sqlite";
	DatabaseAccess _dba;
};

