#include "DatabaseAccess.h"
#include "io.h"
#include "SQLException.h"

const char* DatabaseAccess::DBFILENAME = "galleryDB.sqlite";

DatabaseAccess::DatabaseAccess()
	: _db(nullptr)
{
}

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

void DatabaseAccess::execStatement(const char* sqlStatement) const
{
	char* errmsg = nullptr;
	if (sqlite3_exec(_db, sqlStatement, nullptr, nullptr, &errmsg) != SQLITE_OK)
	{
		throw SQLException(errmsg);
	}
}

void DatabaseAccess::execQuery(const char* sqlStatement, int(*callback)(void*, int, char**, char**), void* callbackData) const
{
	char* errmsg = nullptr;
	if (sqlite3_exec(_db, sqlStatement, callback, callbackData, &errmsg) != SQLITE_OK)
	{
		throw SQLException(errmsg);
	}
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
	catch (const SQLException& e)
	{
		std::cerr << "Error creating database: " << e.what() << std::endl;
	}
}

int DatabaseAccess::albumListDBCallback(void* albumList, int argc, char** argv, char** azColName)
{
	Album album;
	singleAlbumDBCallback(&album, argc, argv, azColName);
	((std::list<Album>*)albumList)->push_back(album);
}

int DatabaseAccess::singleAlbumDBCallback(void* outAlbum, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		const std::string& col = azColName[i];
		if (col == "NAME")
		{
			((Album*)outAlbum)->setName(argv[i]);
		}
		else if (col == "CREATION_DATE")
		{
			((Album*)outAlbum)->setCreationDate(argv[i]);
		}
		else if (col == "USER_ID")
		{
			((Album*)outAlbum)->setOwner(std::stoi(argv[i]));
		}
	}
}

int DatabaseAccess::singleColumnDBCallback(void* out, int argc, char** argv, char** azColName)
{
	*((char**)out) = argv[0];
}

int DatabaseAccess::printDBCallback(void* data, int argc, char** argv, char** azColName)
{
}

const std::list<Album> DatabaseAccess::getAlbums()
{
	auto sql = "SELECT NAME, CREATION_DATE, USER_ID FROM Albums;";
	std::list<Album> ans;
	execQuery(sql, albumListDBCallback, &ans);
	return ans;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	const auto& sql = "SELECT NAME, CREATION_DATE, USER_ID FROM Albums WHERE USER_ID=" + std::to_string(user.getId()) + ';';
	std::list<Album> ans;
	execQuery(sql.c_str(), albumListDBCallback, &ans);
	return ans;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	const auto& sql = "INSERT INTO Albums(NAME, CREATION_DATE, USER_ID) VALUES (\"" + album.getName() 
		+ ", \"" + album.getCreationDate() + "\", " + std::to_string(album.getOwnerId()) + ");";
	execStatement(sql.c_str());
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	const auto& sql = "DELETE FROM Albums WHERE USER_ID=" + std::to_string(userId) +
		" AND NAME=\"" + albumName + "\";";
	execStatement(sql.c_str());
}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	const auto& sql = "SELECT COUNT(1) FROM Albums WHERE NAME=\"" + albumName + "\" AND USER_ID=" + std::to_string(userId) + ';';
	char* countString = nullptr;
	execQuery(sql.c_str(), singleColumnDBCallback, &countString);
	return countString[0] != '0'; // count is greater than zero
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	const auto& sql = "SELECT NAME, CREATION_DATE, USER_ID FROM Albums WHERE NAME=\"" + albumName + "\";";
	Album album;
	execQuery(sql.c_str(), singleAlbumDBCallback, &album);
	return album;
}

void DatabaseAccess::closeAlbum(Album& pAlbum)
{
}

void DatabaseAccess::printAlbums()
{
	std::cout << "Album list:" << std::endl;
	std::cout << "-----------" << std::endl;
	for (const Album& album : getAlbums()) {
		std::cout << std::setw(5) << "* " << album;
	}
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

bool DatabaseAccess::doesUserExists(int userId)
{
	const auto& sql = "SELECT COUNT(1) FROM Users WHERE ID=" + std::to_string(userId) + ';';
	char* countString = nullptr;
	execQuery(sql.c_str(), singleColumnDBCallback, &countString);
	return countString[0] != '0'; // count is greater than zero}
