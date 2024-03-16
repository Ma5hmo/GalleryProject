#pragma once
#include "IDataAccess.h"
#include "sqlite3.h"

class DatabaseAccess : public IDataAccess
{

public:
	DatabaseAccess();
	virtual ~DatabaseAccess() = default;

	// album related
	const std::list<Album> getAlbums() override;
	const std::list<Album> getAlbumsOfUser(const User& user) override;
	void createAlbum(const Album& album) override;
	void deleteAlbum(const std::string& albumName, int userId) override;
	bool doesAlbumExists(const std::string& albumName, int userId) override;
	Album openAlbum(const std::string& albumName) override;
	void closeAlbum(Album& pAlbum) override;
	void printAlbums() override;

	// picture related
	void addPictureToAlbumByName(const std::string& albumName, const Picture& picture) override;
	void removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName) override;
	void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override;
	void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override;

	// user related
	void printUsers() override;
	void createUser(User& user) override;
	void deleteUser(const User& user) override;
	bool doesUserExists(int userId) override;
	User getUser(int userId) override;

	// user statistics
	int countAlbumsOwnedOfUser(const User& user) override;
	int countAlbumsTaggedOfUser(const User& user) override;
	int countTagsOfUser(const User& user) override;
	float averageTagsPerAlbumOfUser(const User& user) override;

	// queries
	User getTopTaggedUser() override;
	Picture getTopTaggedPicture() override;
	std::list<Picture> getTaggedPicturesOfUser(const User& user) override;

	bool open() override;
	void close() override;
	void clear() override;


private:
	void execStatement(const char* sqlStatement) const;
	void execQuery(const char* sqlStatement, int(*callback)(void*, int, char**, char**), void* callbackData) const;
	void createDatabase() const;

	static int albumListDBCallback(void* albumList, int argc, char** argv, char** azColName);
	static int singleAlbumDBCallback(void* outAlbum, int argc, char** argv, char** azColName);
	static int singleColumnDBCallback(void* out, int argc, char** argv, char** azColName);
	static int printDBCallback(void* data, int argc, char** argv, char** azColName);

	static const char* DBFILENAME;
	sqlite3* _db;
};