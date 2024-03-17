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

void DataAccessTest::addRows()
{
	std::cout << "Adding users and albums:" << std::endl;
	for (int i = 1; i <= 3; i++)
	{
		try
		{
			std::cout << "\tCreating user " << i << ":" << std::endl;
			_dba.createUser(User(i, "user" + std::to_string(i)));
			std::cout << "SUCCESS!" << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "FAILED! Error - " << e.what() << std::endl;
		}
		Album album(i, "album" + std::to_string(i));
		album.setCreationDateNow();
		for (int j = 1; j <= 2; j++)
		{
			const auto& id = std::to_string(i) + std::to_string(j);
			Picture pic(std::stoi(id), "pic" + id, "C:/Pictures/" + id + ".png", "");
			pic.setCreationDateNow();
			pic.tagUser(i % 3 + 1);
			pic.tagUser((i % 3 + 2) % 3);
			album.addPicture(pic);
		}
		try
		{
			std::cout << "\tCreating album " << i << ":" << std::endl;
			_dba.createAlbum(album);
			std::cout << "SUCCESS!" << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "FAILED! Error - " << e.what() << std::endl;
		}
	}
}
