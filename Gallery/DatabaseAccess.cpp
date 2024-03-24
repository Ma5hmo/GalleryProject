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
			createDatabase(); // will run "PRAGMA foreign_keys=ON" aswell
		}
		catch (const SQLException& e)
		{
			std::cerr << "Error creating database: " << e.what() << std::endl;
		}
	}
	else
	{
		execStatement("PRAGMA foreign_keys=ON;"); // needs to be run for ON DELETE CASCADE to work
	}
	return true;
}

void DatabaseAccess::close()
{
	if (_db != nullptr)
	{
		sqlite3_close(_db);
		_db = nullptr;
	}
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
	auto createQuery = "CREATE TABLE Users(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,NAME TEXT NOT NULL);"\
		"CREATE TABLE Albums(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,NAME TEXT NOT NULL,CREATION_DATE TEXT NOT NULL,USER_ID INTEGER NOT NULL,FOREIGN KEY(USER_ID) REFERENCES Users(ID) ON DELETE CASCADE);"\
		"CREATE TABLE Pictures(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,NAME TEXT NOT NULL,LOCATION TEXT NOT NULL,CREATION_DATE TEXT NOT NULL,ALBUM_ID INTEGER NOT NULL,FOREIGN KEY(ALBUM_ID) REFERENCES Albums(ID) ON DELETE CASCADE);"\
		"CREATE TABLE Tags(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,PICTURE_ID INTEGER NOT NULL,USER_ID INTEGER NOT NULL,FOREIGN KEY(PICTURE_ID) REFERENCES Pictures(ID) ON DELETE CASCADE,FOREIGN KEY(USER_ID) REFERENCES Users(ID) ON DELETE CASCADE);"\
		"PRAGMA	foreign_keys=ON;";
	execStatement(createQuery);
}

int DatabaseAccess::countQuery(const char* sql) const
{
	int count;
	execQuery(sql, singleIntDBCallback, &count);
	return count;
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
	Picture currPic(0, "");
	for (int i = 0; i < argc; i++)
	{
		if (argv[i] == NULL) // null string
		{
			continue; // column is empty
		}

		const std::string& col = azColName[i];
		if (col == "ANAME")
		{
			album->setName(argv[i]);
		}
		else if (col == "ACD")
		{
			album->setCreationDate(argv[i]);
		}
		else if (col == "AUID")
		{
			album->setOwner(std::stoi(argv[i]));
		}
		else if (col == "PNAME")
		{
			currPic.setName(argv[i]);
		}
		else if (col == "PLOC")
		{
			currPic.setPath(argv[i]);
		}
		else if (col == "PCD")
		{
			currPic.setCreationDate(argv[i]);
		}
		else if (col == "TUID")
		{
			currPic.tagUser(std::stoi(argv[i]));
		}
	}
	if (album->doesPictureExists(currPic.getName()))
	{
		// theres only one picture in currPic since were only looking at a single column
		album->tagUserInPicture(*currPic.getUserTags().begin(), currPic.getName());
	}
	else
	{
		album->addPicture(currPic);
	}
	return 0;
}

int DatabaseAccess::singleFloatDBCallback(void* out, int argc, char** argv, char** azColName)
{
	*((float*)out) = std::stof(argv[0]);
	return 0;
}

int DatabaseAccess::singleIntDBCallback(void* out, int argc, char** argv, char** azColName)
{
	*((int*)out) = std::stoi(argv[0]);
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
	auto sql = "SELECT NAME ANAME, CREATION_DATE ACD, USER_ID AUID FROM Albums;";
	std::list<Album> ans;
	execQuery(sql, albumListDBCallback, &ans);
	return ans;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	const auto& sql = "SELECT NAME ANAME, CREATION_DATE ACD, USER_ID AUID FROM Albums WHERE AUID=" + std::to_string(user.getId()) + ';';
	std::list<Album> ans;
	execQuery(sql.c_str(), albumListDBCallback, &ans);
	return ans;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	auto sql = "INSERT INTO Albums(NAME, CREATION_DATE, USER_ID) VALUES (\"" + album.getName()
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
	// const auto& sql = "SELECT NAME, CREATION_DATE, USER_ID FROM Albums WHERE NAME=\"" + albumName + "\";";
	const auto& sql = "SELECT a.NAME ANAME, a.CREATION_DATE ACD, a.USER_ID AUID, p.NAME PNAME, LOCATION PLOC, p.CREATION_DATE PCD, p.ALBUM_ID PAID, t.USER_ID TUID FROM Albums a "\
		"LEFT JOIN Pictures p ON ALBUM_ID = a.ID LEFT JOIN Tags t ON PICTURE_ID = p.ID "\
		"WHERE ANAME=\"" + albumName + "\";";
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
		+ "\", ID FROM Albums WHERE NAME=\"" + albumName + "\" LIMIT 1;";
	execStatement(sql.c_str());
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	const auto& sql = "DELETE FROM Pictures WHERE ID IN (SELECT p.ID from Pictures p JOIN Albums a\
 ON p.ALBUM_ID=a.ID WHERE p.NAME=\"" + pictureName + "\" AND a.NAME=\"" + albumName + "\");";
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
	const auto& sql = "DELETE FROM Tags WHERE USER_ID=" + std::to_string(userId)
		+ " AND PICTURE_ID IN (SELECT p.ID FROM Pictures p JOIN Albums a ON ALBUM_ID=a.ID WHERE p.NAME=\"" 
		+ pictureName + "\" AND a.NAME=\"" + albumName + "\");";
	execStatement(sql.c_str());
}

void DatabaseAccess::printUsers()
{
	auto sql = "SELECT * FROM Users;";
	execQuery(sql, printUserDBCallback, nullptr);
}

void DatabaseAccess::createUser(User& user)
{
	const auto& sql = "INSERT INTO Users(NAME) VALUES (" + user.getName() + "\");"\
		"SELECT last_insert_rowid()";
	int lastId;
	execQuery(sql.c_str(), singleIntDBCallback, &lastId);
	user.setId(lastId);
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
	const auto& sql = "SELECT COUNT(1) FROM Albums WHERE USER_ID=" + std::to_string(user.getId()) + ';';
	return countQuery(sql.c_str());
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	const auto& sql = "SELECT * FROM Albums a JOIN Pictures p ON a.ID=ALBUM_ID JOIN Tags t ON "\
		"PICTURE_ID=p.ID WHERE t.USER_ID=" + std::to_string(user.getId()) + ';';
	return countQuery(sql.c_str());
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	const auto& sql = "SELECT COUNT(1) FROM Tags WHERE USER_ID=" + std::to_string(user.getId()) + ';';
	return countQuery(sql.c_str());
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	const auto& sql = "SELECT AVG(C) FROM (SELECT COUNT(1) C FROM Albums a JOIN Pictures p ON a.ID=p.ALBUM_ID "\
		"JOIN Tags t ON PICTURE_ID=p.ID WHERE t.USER_ID=" + std::to_string(user.getId()) + " GROUP BY a.ID);";
	float avg;
	execQuery(sql.c_str(), singleFloatDBCallback, &avg);
	return avg;
}
