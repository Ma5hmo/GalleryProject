#include "DatabaseAccess.h"
#include "io.h"

const char* DatabaseAccess::DBFILENAME = "galleryDB.sqlite";

bool DatabaseAccess::open()
{
	int res = sqlite3_open(DBFILENAME, &_db);
	bool fileExists = _access(DBFILENAME, 0) == 0;
	if (res != SQLITE_OK) {
		_db = nullptr;
		return false;
	}
	if (!fileExists)
	{
		createDatabase();
	}
	return true;
}

void DatabaseAccess::close()
{
	sqlite3_close(_db);
	_db = nullptr;
}

void DatabaseAccess::clear()
{
	// TODO
}

bool DatabaseAccess::execStatement(const char* sqlStatement) const
{
	char* errmsg = nullptr;
	if (sqlite3_exec(_db, sqlStatement, nullptr, nullptr, &errmsg) != SQLITE_OK)
	{
		throw std::runtime_error(errmsg);
	}
	return true;
}

void DatabaseAccess::createDatabase() const
{
	const char* usersQuery = "CREATE TABLE Users (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL);";
	const char* albumsQuery = "CREATE TABLE Albums(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, CREATION_DATE TEXT NOT NULL, USER_ID INTEGER NOT NULL, FOREIGN KEY(USER_ID) REFERENCES Users(ID))";
	const char* picturesQuery = "CREATE TABLE Pictures (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,NAME TEXT NOT NULL,LOCATION TEXT NOT NULL,CREATION_DATE TEXT NOT NULL,ALBUMS_ID INTEGER NOT NULL,FOREIGN KEY(ALBUMS_ID) REFERENCES Albums(ID));";
	const char* tagsQuery = "CREATE TABLE Tags (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, PICTURE_ID INTEGER NOT NULL, USER_ID INTEGER NOT NULL, FOREIGN KEY(PICTURE_ID) REFERENCES Pictures(ID), FOREIGN KEY(USER_ID) REFERENCES Users(ID))";
	try {
		execStatement(usersQuery);
		execStatement(albumsQuery);
		execStatement(picturesQuery);
		execStatement(tagsQuery);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error creating database: " << e.what() << std::endl;
	}
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	const auto& sql = "DELETE FROM Albums WHERE USER_ID=" + std::to_string(userId) +
		" AND NAME=" + albumName + ';';
	execStatement(sql.c_str());
}

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	const auto& sql = "INSERT INTO Tags(PICTURE_ID, USER_ID) SELECT Pictures.ID, " + std::to_string(userId)
		+ " FROM Pictures JOIN Albums ON Pictures.ALBUM_ID=Albums.ID WHERE Pictures.NAME = \"" + pictureName
		+ "\" AND Albums.NAME=\"" + albumName + "\";";
	execStatement(sql.c_str());
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	const auto& sql = "DELETE FROM Tags JOIN Pictures ON Pictures.ID=Tags.PICTURE_ID JOIN Albums ON Pictures.ALBUM_ID=Albums.ID WHERE Albums.NAME=\"" + albumName
		+ "\" AND Pictures.NAME=\"" + pictureName + "\" AND Tags.USER_ID=" + std::to_string(userId) + ';';
	execStatement(sql.c_str());
}

void DatabaseAccess::createUser(User& user)
{
	const auto& sql = "INSERT INTO Users(ID, NAME) VALUES (" + std::to_string(user.getId()) + ", \""
		+ user.getName() + "\");";
	execStatement(sql.c_str());
}

void DatabaseAccess::deleteUser(const User& user)
{
	const auto& sql = "DELETE FROM Users WHERE ID=" + std::to_string(user.getId()) + ';';
	execStatement(sql.c_str());
}
