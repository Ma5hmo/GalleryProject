#include "DataAccessTest.h"
#include <cstdio>
#include "SQLException.h"

DataAccessTest::DataAccessTest()
{
	std::remove(_dbFileName);
	_dba = DatabaseAccess(_dbFileName);
}

DataAccessTest::~DataAccessTest()
{
	_dba.close();
}

void DataAccessTest::runTests()
{
	std::cout << "--CREATE TABLES TEST--" << std::endl;
	createTables();

	std::cout << "--ADD ROWS TEST--" << std::endl;
	addRows();

	std::cout << "--UPDATE ROWS TEST--" << std::endl;
	updateRows();

	std::cout << "--DELETE ROWS TEST--" << std::endl;
	removeRows();
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

void DataAccessTest::updateRows()
{
	Picture pic(69, "my femily", "C:/Pictures/myfamily.png", "");
	pic.setCreationDateNow();
	try
	{
		std::cout << "Adding mistaken picture:" << std::endl;
		_dba.addPictureToAlbumByName("album1", pic);
		std::cout << "SUCCESS!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "FAILED! Error - " << e.what() << std::endl;
	}

	try
	{
		std::cout << "Removing mistaken picture:" << std::endl;
		_dba.removePictureFromAlbumByName("album1", "my femily");
		std::cout << "SUCCESS!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "FAILED! Error - " << e.what() << std::endl;
	}

	pic.setName("My Family");

	try
	{
		std::cout << "Adding working picture:" << std::endl;
		_dba.addPictureToAlbumByName("album1", pic);
		std::cout << "SUCCESS!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "FAILED! Error - " << e.what() << std::endl;
	}
}

void DataAccessTest::removeRows()
{
	try
	{
		std::cout << "Deleting album2:" << std::endl;
		_dba.deleteAlbum("album2", 2);
		if (_dba.doesAlbumExists("album2", 2) == true)
		{
			throw SQLException("album still exists after deletion");
		}
		std::cout << "SUCCESS!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "FAILED! Error - " << e.what() << std::endl;
	}

	try
	{
		std::cout << "Deleting user2:" << std::endl;
		_dba.deleteUser(User(2, "user2"));
		std::cout << "SUCCESS!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "FAILED! Error - " << e.what() << std::endl;
	}
}
