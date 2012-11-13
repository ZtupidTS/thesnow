#ifndef __DIRECTORYCACHE_H__
#define __DIRECTORYCACHE_H__

/*
This class is the directory cache used to store retrieved directory listings
for further use.
Directory get either purged from the cache if the maximum cache time exceeds,
or on possible data inconsistencies.
For example since some servers are case sensitive and others aren't, a
directory is removed from cache once an operation effects a file wich matches
multiple entries in a cache directory using a case insensitive search
On other operations, the directory is marked as unsure. It may still be valid,
but for some operations the engine/interface prefers to retrieve a clean
version.
*/

const int CACHE_TIMEOUT = 1800; // In seconds

class CDirectoryCache
{
public:
	enum Filetype
	{
		unknown,
		file,
		dir
	};

	CDirectoryCache();
	~CDirectoryCache();

	void Store(const CDirectoryListing &listing, const CServer &server);
	bool GetChangeTime(CTimeEx& time, const CServer &server, const CServerPath &path);
	bool Lookup(CDirectoryListing &listing, const CServer &server, const CServerPath &path, bool allowUnsureEntries, bool& is_outdated);
	bool DoesExist(const CServer &server, const CServerPath &path, int &hasUnsureEntries, bool &is_outdated);
	bool LookupFile(CDirentry &entry, const CServer &server, const CServerPath &path, const wxString& file, bool &dirDidExist, bool &matchedCase);
	bool InvalidateFile(const CServer &server, const CServerPath &path, const wxString& filename, bool *wasDir = 0);
	bool UpdateFile(const CServer &server, const CServerPath &path, const wxString& filename, bool mayCreate, enum Filetype type = file, wxLongLong size = -1);
	bool RemoveFile(const CServer &server, const CServerPath &path, const wxString& filename);
	void InvalidateServer(const CServer& server);
	void RemoveDir(const CServer& server, const CServerPath& path, const wxString& filename, const CServerPath& target);
	void Rename(const CServer& server, const CServerPath& pathFrom, const wxString& fileFrom, const CServerPath& pathTo, const wxString& fileTo);

protected:

	class CCacheEntry
	{
	public:
		CCacheEntry() : lruIt() { };
		CCacheEntry(const CCacheEntry &entry);
		~CCacheEntry() { };
		CDirectoryListing listing;
		CTimeEx modificationTime;

		CCacheEntry& operator=(const CCacheEntry &a);

		void* lruIt; // void* to break cyclic declaration dependency
	};

	class CServerEntry
	{
	public:
		CServer server;
		std::list<CCacheEntry> cacheList;
	};

	typedef std::list<CServerEntry>::iterator tServerIter;

	tServerIter CreateServerEntry(const CServer& server);
	tServerIter GetServerEntry(const CServer& server);

	typedef std::list<CCacheEntry>::iterator tCacheIter;
	typedef std::list<CCacheEntry>::const_iterator tCacheConstIter;

	bool Lookup(tCacheIter &cacheIter, tServerIter &sit, const CServerPath &path, bool allowUnsureEntries, bool& is_outdated);

	static std::list<CServerEntry> m_serverList;

	static int m_nRefCount;

	void UpdateLru(tServerIter const& sit, tCacheIter const& cit);

	void Prune();

	typedef std::pair<tServerIter, tCacheIter> tFullEntryPosition;
	typedef std::list<tFullEntryPosition> tLruList;
	static tLruList m_leastRecentlyUsedList;

	static wxLongLongNative m_totalFileCount;
};

#endif
