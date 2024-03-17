#include "DatabaseAccess.h"
#include "io.h"
#include <vector>
#include <algorithm>

#include "AlbumNotOpenException.h"
#include "ItemNotFoundException.h"
#include "SQLException.h"

DatabaseAccess::DatabaseAccess()
	: _db(nullptr), _dbFileName("galleryDB.sqlite")
{
}

DatabaseAccess::DatabaseAccess(const char* DBFileName)
	: _db(nullptr), _dbFileName(DBFileName)
{
}

User DatabaseAccess::getTopTaggedUser()
{
	User user(0, "");
	auto sql = "SELECT Users.ID ID, Users.NAME NAME FROM Tags JOIN Users on USER_ID=Users.ID GROUP BY Users.ID ORDER BY COUNT(1) DESC LIMIT 1;";
	execQuery(sql, singleUserDBCallback, &user);
	return user;
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	Picture p(0, "");
	auto sql = "SELECT * FROM Tags JOIN Pictures ON Pictures.ID=PICTURE_ID WHERE PICTURE_ID=(SELECT PICTURE_ID FROM Tags GROUP BY PICTURE_ID ORDER BY COUNT(1) DESC LIMIT 1);";
	execQuery(sql, singlePictureDBCallback, &p);
	return p;
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	const auto& sql = "SELECT * FROM Pictures JOIN Tags ON Pictures.ID=PICTURE_ID WHERE Tags.USER_ID=" + std::to_string(user.getId()) + ';';
	std::list<Picture> ans;
	execQuery(sql.c_str(), pictureListDBCallback, &ans);
	return ans;
}

bool DatabaseAccess::open()
{
	bool fileExists = _access(_dbFileName, 0) == 0;
	int res = sqlite3_open(_dbFileName, &_db);
	if (res != SQLITE_OK) {
		_db = nullptr;
		throw SQLException("Error opening database");
	}
	if (!fileExists)
	{
		try
		{
			createDatabase();
		}
		catch (const SQLException& e)
		{
			std::cerr << "Error creating database: " << e.what() << std::endl;
		}
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
	const char* usersQuery = "CREATE TABLE Users(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL);";
	const char* albumsQuery = "CREATE TABLE Albums(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, CREATION_DATE TEXT NOT NULL, USER_ID INTEGER NOT NULL, FOREIGN KEY(USER_ID) REFERENCES Users(ID))";
	const char* picturesQuery = "CREATE TABLE Pictures(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,NAME TEXT NOT NULL,LOCATION TEXT NOT NULL,CREATION_DATE TEXT NOT NULL,ALBUMS_ID INTEGER NOT NULL,FOREIGN KEY(ALBUMS_ID) REFERENCES Albums(ID));";
	const char* tagsQuery = "CREATE TABLE Tags(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, PICTURE_ID INTEGER NOT NULL, USER_ID INTEGER NOT NULL, FOREIGN KEY(PICTURE_ID) REFERENCES Pictures(ID), FOREIGN KEY(USER_ID) REFERENCES Users(ID))";

	execStatement(usersQuery);
	execStatement(albumsQuery);
	execStatement(picturesQuery);
	execStatement(tagsQuery);
}

int DatabaseAccess::countQuery(const char* sql) const
{
	std::string countString;
	execQuery(sql, singleColumnDBCallback, &countString);
	if (countString.empty())
	{
		throw MyException("Error counting albums");
	}
	std::cout << "NIGGER " << countString << std::endl;
	return std::stoi(countString);
}

int DatabaseAccess::albumListDBCallback(void* albumList, int argc, char** argv, char** azColName)
{
	Album album;
	singleAlbumDBCallback(&album, argc, argv, azColName);
	((std::list<Album>*)albumList)->push_back(album);
	return 0;
}

int DatabaseAccess::singleAlbumDBCallback(void* outAlbum, int argc, char** argv, char** azColName)
{
	Album* album = (Album*)outAlbum;
	for (int i = 0; i < argc; i++)
	{
		const std::string& col = azColName[i];
		if (col == "NAME")
		{
			album->setName(argv[i]);
		}
		else if (col == "CREATION_DATE")
		{
			album->setCreationDate(argv[i]);
		}
		else if (col == "USER_ID")
		{
			album->setOwner(std::stoi(argv[i]));
		}
	}
	return 0;
}

int DatabaseAccess::singleColumnDBCallback(void* out, int argc, char** argv, char** azColName)
{
	*((std::string*)out) = argv[0];
	return 0;
}

int DatabaseAccess::printUserDBCallback(void*, int argc, char** argv, char** azColName)
{
	User user(0, "");
	singleUserDBCallback(&user, argc, argv, azColName);
	std::cout << user << std::endl;
	return 0;
}

int DatabaseAccess::singleUserDBCallback(void* outUser, int argc, char** argv, char** azColName)
{
	User* user = (User*)outUser;
	for (int i = 0; i < argc; i++)
	{
		const std::string& col = azColName[i];
		if (col == "NAME")
		{
			user->setName(argv[i]);
		}
		else if (col == "ID")
		{
			user->setId(std::stoi(argv[i]));
		}
	}
	return 0;
}

int DatabaseAccess::singlePictureDBCallback(void* outPicture, int argc, char** argv, char** azColName)
{
	Picture* pic = (Picture*)outPicture;
	for (int i = 0; i < argc; i++)
	{
		const std::string& col = azColName[i];
		if (col == "NAME")
		{
			pic->setName(argv[i]);
		}
		else if (col == "ID")
		{
			pic->setId(std::stoi(argv[i]));
		}
		else if (col == "CREATION_DATE")
		{
			pic->setCreationDate(argv[i]);
		}
		else if (col == "LOCATION")
		{
			pic->setPath(argv[i]);
		}
		else if (col == "USER_ID")
		{
			pic->tagUser(std::stoi(argv[i]));
		}
	}
	return 0;
}

int DatabaseAccess::pictureListDBCallback(void* pictureList, int argc, char** argv, char** azColName)
{
	Picture p(0, "");
	singlePictureDBCallback(&p, argc, argv, azColName);
	((std::list<Picture>*) pictureList)->push_back(p);
	return 0;
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
		+ "\", \"" + album.getCreationDate() + "\", " + std::to_string(album.getOwnerId()) + ");";
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
	return countQuery(sql.c_str()) > 0;
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	//	const auto& sql = "SELECT * FROM Tags JOIN Pictures ON PICTURE_ID=Pictures.id JOIN Albums ON ALBUM_ID=Albums.ID WHERE Albums.NAME=\"" + albumName + "\";";
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

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	const auto& sql = "INSERT INTO Pictures(NAME, LOCATION, CREATION_DATE, ALBUM_ID) SELECT \"" + picture.getName()
		+ "\", \"" + picture.getPath() + "\", \"" + picture.getCreationDate()
		+ "\", ID FROM Albums where NAME=\"" + albumName + "\";";
	execStatement(sql.c_str());
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	const auto& sql = "DELETE FROM Pictures JOIN Albums ON ALBUM_ID=Albums.ID WHERE Albums.NAME=\"" + albumName
		+ "\" AND Pictures.NAME=\"" + pictureName + "\";";
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

void DatabaseAccess::printUsers()
{
	auto sql = "SELECT * FROM Users;";
	execQuery(sql, printUserDBCallback, nullptr);
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
	return countQuery(sql.c_str()) > 0;
}

User DatabaseAccess::getUser(int userId)
{
	const auto& sql = "SELECT NAME FROM Users WHERE ID=" + std::to_string(userId) + ';';
	User user(userId, "");
	execQuery(sql.c_str(), singleUserDBCallback, &user);
	if (user.getName().empty())
	{
		throw ItemNotFoundException("User", userId);
	}
	return user;
}

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	const auto& sql = "SELECT COUNT(1) FROM WHERE USER_ID=" + std::to_string(user.getId()) + ';';
	return countQuery(sql.c_str());
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	const auto& sql = "SELECT * FROM Albums JOIN PICTURES ON Albums.ID=ALBUM_ID JOIN Tags ON"\
		"PICTURE_ID=Pictures.ID WHERE Tags.USER_ID=" + std::to_string(user.getId()) + ';';
	return countQuery(sql.c_str());
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	const auto& sql = "SELECT COUNT(1) FROM Tags WHERE USER_ID=" + std::to_string(user.getId()) + ';';
	return countQuery(sql.c_str());
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	const auto& sql = "SELECT AVG(C) FROM (SELECT COUNT(1) C FROM Albums JOIN Pictures ON Albums.ID=Pictures.ALBUM_ID"\
		"JOIN Tags ON PICTURE_ID = Pictures.ID WHERE Tags.USER_ID = " + std::to_string(user.getId()) + "GROUP BY Albums.ID);";
	std::string avgString;
	execQuery(sql.c_str(), singleColumnDBCallback, &avgString);
	if (avgString.empty())
	{
		throw MyException("Error getting average");
	}
	return std::stof(avgString);
}
