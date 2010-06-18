/* static char *fastcopy_id = 
	"@(#)Copyright (C) 2004-2010 H.Shirouzu		fastcopy.h	Ver2.00"; */
/* ========================================================================
	Project  Name			: Fast Copy file and directory
	Create					: 2004-09-15(Wed)
	Update					: 2010-05-10(Mon)
	Copyright				: H.Shirouzu
	Reference				: 
	======================================================================== */

#include "tlib/tlib.h"
#include "resource.h"
#include "utility.h"
#include "regexp.h"

#define MIN_SECTOR_SIZE		(512)
#define OPT_SECTOR_SIZE		(2048)
#define MAX_BUF				(1024 * 1024 * 1024)
#define MIN_BUF				(1024 * 1024)
#define BIGTRANS_ALIGN		(32 * 1024)
#define APP_MEMSIZE			(6 * 1024 * 1024)
#define PATH_LOCAL_PREFIX	L"\\\\?\\"
#define PATH_UNC_PREFIX		L"\\\\?\\UNC"
#define PATH_LOCAL_PREFIX_LEN	4
#define PATH_UNC_PREFIX_LEN		7

#define MAX_BASE_BUF		( 512 * 1024 * 1024)
#define MIN_BASE_BUF		(  64 * 1024 * 1024)
#define RESERVE_BUF			( 128 * 1024 * 1024)

#define MIN_ATTR_BUF		(256 * 1024)
//#define MAX_ATTR_BUF		(128 * 1024 * 1024)
#define MIN_ATTRIDX_BUF		ALIGN_SIZE((MIN_ATTR_BUF / 4), PAGE_SIZE)
#define MAX_ATTRIDX_BUF(x)	ALIGN_SIZE(((x) / 32), PAGE_SIZE)
#define MIN_MKDIRQUEUE_BUF	(8 * 1024)
#define MAX_MKDIRQUEUE_BUF	(256 * 1024)
#define MIN_DSTDIREXT_BUF	(64 * 1024)
#define MAX_DSTDIREXT_BUF	(2 * 1024 * 1024)
#define MIN_ERR_BUF			(64 * 1024)
#define MAX_ERR_BUF			(4 * 1024 * 1024)
#define MAX_LIST_BUF		(128 * 1024)
#define MIN_PUTLIST_BUF		(1 * 1024 * 1024)
#define MAX_PUTLIST_BUF		(4 * 1024 * 1024)

#define MIN_DIGEST_LIST		(1 * 1024 * 1024)
#define MAX_DIGEST_LIST		(8 * 1024 * 1024)
#define MIN_MOVEPATH_LIST	(1 * 1024 * 1024)
#define MAX_MOVEPATH_LIST	(8 * 1024 * 1024)

#define CV_WAIT_TICK		1000

#define FASTCOPY_MUTEX		"FastCopyMutex"
#define FASTCOPY_EVENT		"FastCopyEvent"

#define FASTCOPY_ERROR_EVENT	0x01
#define FASTCOPY_STOP_EVENT		0x02

struct TotalTrans {
	BOOL	isPreSearch;
	int		preDirs;
	int		preFiles;
	_int64	preTrans;
	int		readDirs;
	int		readFiles;
	int		readAclStream;
	_int64	readTrans;
	int		writeDirs;
	int		writeFiles;
	int		linkFiles;
	int		writeAclStream;
	_int64	writeTrans;
	int		verifyFiles;
	_int64	verifyTrans;
	int		deleteDirs;
	int		deleteFiles;
	_int64	deleteTrans;
	int		skipDirs;
	int		skipFiles;
	_int64	skipTrans;
	int		filterSrcSkips;
	int		filterDelSkips;
	int		filterDstSkips;
	int		errDirs;
	int		errFiles;
	_int64	errTrans;
	_int64	errStreamTrans;
	int		errAclStream;
	int		openRetry;
};

struct TransInfo {
	TotalTrans			total;
	DWORD				tickCount;
	BOOL				isSameDrv;
	DWORD				ignoreEvent;
	DWORD				waitTick;

	VBuf				*errBuf;
	CRITICAL_SECTION	*errCs;

	VBuf				*listBuf;
	CRITICAL_SECTION	*listCs;

	WCHAR				curPath[MAX_PATH_EX];
};

struct FileStat {
	_int64		fileID;
	FILETIME	ftCreationTime;
	FILETIME	ftLastAccessTime;
	FILETIME	ftLastWriteTime;
	DWORD		nFileSizeLow;	// WIN32_FIND_DATA �� nFileSizeLow/High
	DWORD		nFileSizeHigh;	// �Ƃ͋t���i_int64 �p�j
	DWORD		dwFileAttributes;	// 0 == ALTSTREAM
	HANDLE		hFile;
	DWORD		lastError;
	int			renameCount;
	BOOL		isExists;
	BOOL		isCaseChanged;
	int			size;
	int			minSize;		// upperName �����܂߂Ȃ�
	BYTE		*upperName;		// cFileName �I�[+1���w��
	DWORD		hashVal;		// upperName �� hash�l

	// extraData
	BYTE		*acl;
	int			aclSize;
	BYTE		*ead;
	int			eadSize;
	BYTE		*rep;	// reparse data
	int			repSize;

	// md5/sha1 digest
	BYTE		digest[SHA1_SIZE];

	FileStat	*next;			// for hashTable
	BYTE		cFileName[4];	// 4 == dummy

	_int64	FileSize() { return *(_int64 *)&nFileSizeLow; }
	void	SetFileSize(_int64 file_size) { *(_int64 *)&nFileSizeLow = file_size; }
	void	SetLinkData(DWORD *data) { memcpy(digest, data, sizeof(DWORD) * 3); }
	DWORD	*GetLinkData() { return (DWORD *)digest; }
	_int64	CreateTime() { return *(_int64 *)&ftCreationTime;   }
	_int64	AccessTime() { return *(_int64 *)&ftLastAccessTime; }
	_int64	WriteTime()  { return *(_int64 *)&ftLastWriteTime; }
};

class StatHash {
	FileStat	**hashTbl;
	int			hashNum;
	int			HashNum(int data_num);

public:
	StatHash() {}
	int		RequireSize(int data_num) { return	sizeof(FileStat *) * HashNum(data_num); }
	BOOL	Init(FileStat **data, int _data_num, void *tbl_buf);
	FileStat *Search(void *upperName, DWORD hash_val);
};

struct DirStatTag {
	int			statNum;
	int			oldStatSize;
	FileStat	*statBuf;
	FileStat	**statIndex;
};

struct LinkObj : public THashObj {
	void	*path;
	int		len;
	DWORD	data[3];
	DWORD	nLinks;
	LinkObj(const void *_path, DWORD _nLinks, DWORD *_data, int len=-1) {
		if (len == -1) len = strlenV(_path) + 1;
		int alloc_len = len * CHAR_LEN_V;
		path = malloc(alloc_len);
		memcpy(path, _path, alloc_len);
		memcpy(data, _data, sizeof(data));
		nLinks = _nLinks;
	}
	~LinkObj() { if (path) free(path); }
};

class TLinkHashTbl : public THashTbl {
public:
	virtual BOOL IsSameVal(THashObj *obj, const void *_data) {
		return	!memcmp(&((LinkObj *)obj)->data, _data, sizeof(((LinkObj *)obj)->data));
	}

public:
	TLinkHashTbl() : THashTbl() {}
	virtual ~TLinkHashTbl() {}
	virtual BOOL Init(int _num) { return THashTbl::Init(_num); }
	u_int	MakeHashId(DWORD volSerial, DWORD highIdx, DWORD lowIdx) {
		DWORD data[] = { volSerial, highIdx, lowIdx };
		return	MakeHashId(data);
	}
	u_int	MakeHashId(DWORD *data) { return MakeHash(data, sizeof(DWORD) * 3); }
};

inline _int64 WriteTime(const WIN32_FIND_DATAW &fdat) {
	return *(_int64 *)&fdat.ftLastWriteTime;
}
inline _int64 FileSize(const WIN32_FIND_DATAW &fdat) {
	return ((_int64)fdat.nFileSizeHigh << 32) + fdat.nFileSizeLow;
}
inline _int64 FileSize(const BY_HANDLE_FILE_INFORMATION &bhi) {
	return ((_int64)bhi.nFileSizeHigh << 32) + bhi.nFileSizeLow;
}

class FastCopy {
public:
	enum Mode { DIFFCP_MODE, SYNCCP_MODE, MOVE_MODE, MUTUAL_MODE, DELETE_MODE };
	enum OverWrite { BY_NAME, BY_ATTR, BY_LASTEST, BY_CONTENTS, BY_ALWAYS };
	enum FsType { FSTYPE_NONE, FSTYPE_NTFS, FSTYPE_FAT, FSTYPE_NETWORK };
	enum Flags {
				CREATE_OPENERR_FILE	=	0x00000001,
				USE_OSCACHE_READ	=	0x00000002,
				USE_OSCACHE_WRITE	=	0x00000004,
				PRE_SEARCH			=	0x00000008,
				SAMEDIR_RENAME		=	0x00000010,
				SKIP_EMPTYDIR		=	0x00000020,
				FIX_SAMEDISK		=	0x00000040,
				FIX_DIFFDISK		=	0x00000080,
				AUTOSLOW_IOLIMIT	=	0x00000100,
				WITH_ACL			=	0x00000200,
				WITH_ALTSTREAM		=	0x00000400,
				OVERWRITE_DELETE	=	0x00000800,
				OVERWRITE_DELETE_NSA=	0x00001000,
				FILE_REPARSE		=	0x00002000,
				DIR_REPARSE			=	0x00004000,
				COMPARE_CREATETIME	=	0x00008000,
				SERIAL_MOVE			=	0x00010000,
				SERIAL_VERIFY_MOVE	=	0x00020000,
				USE_OSCACHE_READVERIFY=	0x00040000,
				RESTORE_HARDLINK	=	0x00080000,
				DISABLE_COMPARE_LIST=	0x00100000,
				DEL_BEFORE_CREATE	=	0x00200000,
				DELDIR_WITH_FILTER	=	0x00400000,
				LISTING				=	0x01000000,
				VERIFY_MD5			=	0x02000000,
				OVERWRITE_PARANOIA	=	0x04000000,
				VERIFY_FILE			=	0x08000000,
				LISTING_ONLY		=	0x10000000,
				REPORT_ACL_ERROR	=	0x20000000,
				REPORT_STREAM_ERROR	=	0x40000000
			};
	enum SpecialWaitTick { SUSPEND_WAITTICK=0x7fffffff };

	struct Info {
		DWORD	ignoreEvent;	// (I/ )
		Mode	mode;			// (I/ )
		OverWrite overWrite;	// (I/ )
		BOOL	isPhysLock;		// (I/ )
		int		flags;			// (I/ )
		int		bufSize;		// (I/ )
		int		maxOpenFiles;	// (I/ )
		int		maxTransSize;	// (I/ )
		int		maxAttrSize;	// (I/ )
		int		maxDirSize;		// (I/ )
		int		nbMinSizeNtfs;	// (I/ ) FILE_FLAG_NO_BUFFERING �ŃI�[�v������ŏ��T�C�Y
		int		nbMinSizeFat;	// (I/ ) FILE_FLAG_NO_BUFFERING �ŃI�[�v������ŏ��T�C�Y (FAT�p)
		int		maxLinkHash;	// (I/ ) Dest Hardlink �p hash table �T�C�Y
		_int64	allowContFsize;	// (I/ )
		HWND	hNotifyWnd;		// (I/ )
		UINT	uNotifyMsg;		// (I/ )
		int		lcid;			// (I/ )
		_int64	fromDateFilter;	// (I/ ) �ŌÓ����t�B���^
		_int64	toDateFilter;	// (I/ ) �ŐV�����t�B���^
		_int64	minSizeFilter;	// (I/ ) �Œ�T�C�Y�t�B���^
		_int64	maxSizeFilter;	// (I/ ) �ő�T�C�Y�t�B���^
		char	driveMap[64];	// (I/ ) �����h���C�u�}�b�v
		BOOL	isRenameMode;	// ( /O) ...�u�������܂��v�_�C�A���O�^�C�g���p���i�b��j
	};							//			 �����I�ɁA��񂪑�����΁A�����o����؂藣��

	enum Notify { END_NOTIFY, CONFIRM_NOTIFY, RENAME_NOTIFY, LISTING_NOTIFY };
	struct Confirm {
		enum Result { CANCEL_RESULT, IGNORE_RESULT, CONTINUE_RESULT };
		const void	*message;		// (I/ )
		BOOL		allow_continue;	// (I/ )
		const void	*path;			// (I/ )
		DWORD		err_code;		// (I/ )
		Result		result;			// ( /O)
	};

public:
	FastCopy(void);
	virtual ~FastCopy();

	BOOL RegisterInfo(const PathArray *_srcArray, const PathArray *_dstArray, Info *_info,
			const PathArray *_includeArray=NULL, const PathArray *_excludeArray=NULL);
	BOOL TakeExclusivePriv(void);
	BOOL AllocBuf(void);
	BOOL Start(TransInfo *ti);
	BOOL End(void);
	BOOL IsStarting(void) { return hReadThread || hWriteThread; }
	BOOL Suspend(void);
	BOOL Resume(void);
	void Aborting(void);
	BOOL WriteErrLog(void *message, int len=-1);
	BOOL IsAborting(void) { return isAbort; }
	void SetWaitTick(DWORD wait) { waitTick = wait; }
	DWORD GetWaitTick() { return waitTick; }
	BOOL GetTransInfo(TransInfo *ti, BOOL fullInfo=TRUE);

protected:
	enum Command {
		WRITE_FILE, WRITE_BACKUP_FILE, WRITE_FILE_CONT, WRITE_ABORT, WRITE_BACKUP_ACL,
		WRITE_BACKUP_EADATA, WRITE_BACKUP_ALTSTREAM, WRITE_BACKUP_END, CASE_CHANGE,
		CREATE_HARDLINK, REMOVE_FILES, MKDIR, INTODIR, DELETE_FILES, RETURN_PARENT, REQ_EOF
	};
	enum PutListOpt {
			PL_NORMAL=0x1, PL_DIRECTORY=0x2, PL_HARDLINK=0x4, PL_REPARSE=0x8,
			PL_CASECHANGE=0x10, PL_DELETE=0x20,
			PL_COMPARE=0x10000, PL_ERRMSG=0x20000, PL_NOADD=0x40000,
	};
	enum LinkStatus { LINK_ERR=0, LINK_NONE=1, LINK_ALREADY=2, LINK_REGISTER=3 };
	enum ConfirmErrFlags { CEF_STOP=0x0001, CEF_NOAPI=0x0002 };

	struct ReqHeader : public TListObj {	// request header
		Command		command;
		BYTE		*buf;
		int			bufSize;
		int			reqSize;
		FileStat	stat;	// �ϒ�
	};
	struct ReqBuf {
		BYTE		*buf;
		int			bufSize;
		ReqHeader	*req;
		int			reqSize;
	};

	struct MoveObj {
		_int64		fileID;
		_int64		fileSize;
		enum		Status { START, DONE, ERR } status;
		DWORD		dwFileAttributes;
		BYTE		path[1];
	};
	struct DigestObj {
		_int64		fileID;
		_int64		fileSize;
		DWORD		dwFileAttributes;
		BYTE		digest[SHA1_SIZE];
		int			pathLen;
		BYTE		path[1];
	};
	struct DigestCalc {
		_int64		fileID;
		enum		Status { INIT, CONT, DONE, ERR, FIN };
		DWORD		status;
		int			dataSize;
		BYTE		digest[SHA1_SIZE];
		BYTE		*data;
		BYTE		path[1]; // ����� dstSector���E��Ƀf�[�^������
	};

	struct RandomDataBuf {	// �㏑���폜�p
		BOOL	is_nsa;
		int		base_size;
		int		buf_size;
		BYTE	*buf[3];
	};

	class TReqList : public TList {	// ���N�G�X�g�L���[
	public:
		TReqList(void) {}
		ReqHeader *TopObj(void) { return (ReqHeader *)TList::TopObj(); }
		ReqHeader *NextObj(ReqHeader *obj) { return (ReqHeader *)TList::NextObj(obj); }
	};

	// ��{���
	DriveMng	driveMng;	// Drive ���
	Info		info;		// �I�v�V�����w�蓙
	StatHash	hash;
	PathArray	srcArray;
	PathArray	dstArray;

	void	*src;			// src �p�X�i�[�p
	void	*dst;			// dst �p�X�i�[�p
	void	*confirmDst;	// �㏑���m�F�����p
	void	*hardLinkDst;	// �n�[�h�����N�p
	int		srcBaseLen;		// src �p�X�̌Œ蕔���̒���
	int		dstBaseLen;		// dst �p�X�̌Œ蕔���̒���
	int		srcPrefixLen;	// \\?\ or \\?\UNC\ �̒���
	int		dstPrefixLen;
	BOOL	isExtendDir;
	BOOL	isMetaSrc;
	BOOL	isListing;
	BOOL	isListingOnly;
	int		maxStatSize;	// max size of FileStat
	int		nbMinSize;		// struct Info �Q��
	BOOL	enableAcl;
	BOOL	enableStream;

	// �Z�N�^���Ȃ�
	int		srcSectorSize;
	int		dstSectorSize;
	int		sectorSize;
	DWORD	maxReadSize;
	DWORD	maxDigestReadSize;
	DWORD	maxWriteSize;
	FsType	srcFsType;
	FsType	dstFsType;
	BYTE	src_root[MAX_PATH];

	TotalTrans	total;		// �t�@�C���R�s�[���v���

	// filter
	enum		{ REG_FILTER=0x1, DATE_FILTER=0x2, SIZE_FILTER=0x4 };
	DWORD		filterMode;
	enum		{ FILE_EXP, DIR_EXP, MAX_FTYPE_EXP };
	enum		{ INC_EXP, EXC_EXP, MAX_KIND_EXP };
	RegExpEx	regExp[MAX_FTYPE_EXP][MAX_KIND_EXP];

	// �o�b�t�@
	VBuf	mainBuf;		// Read/Write �p buffer
//	VBuf	baseBuf;		// mainBuf �ȊO�̐ebuffer
	VBuf	fileStatBuf;	// src file stat �p buffer
	VBuf	dirStatBuf;		// src dir stat �p buffer
	VBuf	dstStatBuf;		// dst dir/file stat �p buffer
	VBuf	dstStatIdxBuf;	// dstStatBuf �� entry �� index sort �p
	VBuf	mkdirQueueBuf;
	VBuf	dstDirExtBuf;
	VBuf	srcDigestBuf;
	VBuf	dstDigestBuf;
	VBuf	errBuf;
	VBuf	listBuf;

	// �f�[�^�]���L���[�֘A
	TReqList	readReqList;
	TReqList	writeReqList;
	TReqList	rDigestReqList;
	BYTE		*usedOffset;
	BYTE		*freeOffset;
	ReqHeader	*writeReq;
	_int64		nextFileID;
	_int64		errFileID;
	FileStat	**openFiles;
	int			openFilesCnt;

	// �X���b�h�֘A
	HANDLE		hReadThread;
	HANDLE		hWriteThread;
	HANDLE		hRDigestThread;
	HANDLE		hWDigestThread;
	int			nThreads;
	HANDLE		hRunMutex;
	Condition	cv;

	CRITICAL_SECTION errCs;
	CRITICAL_SECTION listCs;

	// ���ԏ��
	DWORD	startTick;
	DWORD	endTick;
	DWORD	suspendTick;
	volatile DWORD	waitTick;

	// ���[�h�E�t���O��
	BOOL	isAbort;
	BOOL	isSuspend;
	BOOL	isSameDrv;
	BOOL	isSameVol;
	BOOL	isRename;
	enum	DstReqKind { DSTREQ_NONE=0, DSTREQ_READSTAT=1, DSTREQ_DIGEST=2 } dstAsyncRequest;
	BOOL	dstRequestResult;
	enum	RunMode { RUN_NORMAL, RUN_DIGESTREQ } runMode;

	// �_�C�W�F�X�g�֘A
	TDigest		srcDigest;
	TDigest		dstDigest;
	BYTE		srcDigestVal[SHA1_SIZE];
	BYTE		dstDigestVal[SHA1_SIZE];

	DataList	digestList;	// �n�b�V��/Open�L�^
	BOOL IsUsingDigestList() {
		return (info.flags & VERIFY_FILE) && (info.flags & LISTING_ONLY) == 0;
	}
	enum		CheckDigestMode { CD_NOWAIT, CD_WAIT, CD_FINISH };
	DataList	wDigestList;

	// �ړ��֘A
	DataList		moveList;		// �ړ�
	DataList::Head	*moveFinPtr;	// �������ݏI��ID�ʒu

	TLinkHashTbl	hardLinkList;	// �n�[�h�����N�p���X�g

	static unsigned WINAPI ReadThread(void *fastCopyObj);
	static unsigned WINAPI WriteThread(void *fastCopyObj);
	static unsigned WINAPI DeleteThread(void *fastCopyObj);
	static unsigned WINAPI RDigestThread(void *fastCopyObj);
	static unsigned WINAPI WDigestThread(void *fastCopyObj);

	BOOL ReadThreadCore(void);
	BOOL DeleteThreadCore(void);
	BOOL WriteThreadCore(void);
	BOOL RDigestThreadCore(void);
	BOOL WDigestThreadCore(void);

	LinkStatus CheckHardLink(void *path, int len=-1, HANDLE hFileOrg=INVALID_HANDLE_VALUE,
			DWORD *data=NULL);
	BOOL RestoreHardLinkInfo(DWORD *link_data, void *path, int base_len);

	BOOL CheckAndCreateDestDir(int dst_len);
	BOOL FinishNotify(void);
	BOOL PreSearch(void);
	BOOL PreSearchProc(void *path, int prefix_len, int dir_len);
	BOOL PutMoveList(_int64 fileID, void *path, int path_len, _int64 file_size, DWORD attr,
			MoveObj::Status status);
	BOOL FlushMoveList(BOOL is_finish=FALSE);
	BOOL ReadProc(int dir_len, BOOL confirm_dir=TRUE);
	BOOL GetDirExtData(ReqBuf *req_buf, FileStat *stat);
	BOOL IsOverWriteFile(FileStat *srcStat, FileStat *dstStat);
	int  MakeRenameName(void *new_fname, int count, void *org_fname);
	BOOL SetRenameCount(FileStat *stat);
	BOOL FilterCheck(const void *path, const void *fname, DWORD attr, _int64 write_time,
			_int64 file_size);
	BOOL ReadDirEntry(int dir_len, BOOL confirm_dir);
	HANDLE CreateFileWithRetry(void *path, DWORD mode, DWORD share, SECURITY_ATTRIBUTES *sa,
			DWORD cr_mode, DWORD flg, HANDLE hTempl, int retry_max=10);
	BOOL OpenFileProc(FileStat *stat, int dir_len);
	BOOL OpenFileBackupProc(FileStat *stat, int src_len);
	BOOL ReadMultiFilesProc(int dir_len);
	BOOL CloseMultiFilesProc(int maxCnt=0);
	void *RestoreOpenFilePath(void *path, int idx, int dir_len);
	BOOL ReadFileWithReduce(HANDLE hFile, void *buf, DWORD size, DWORD *written,
			OVERLAPPED *overwrap);
	BOOL ReadFileProc(int start_idx, int *end_idx, int dir_len);
	BOOL DeleteProc(void *path, int dir_len);
	BOOL DeleteDirProc(void *path, int dir_len, void *fname, FileStat *stat);
	BOOL DeleteFileProc(void *path, int dir_len, void *fname, FileStat *stat);

	void SetupRandomDataBuf(void);
	void GenRandomName(void *path, int fname_len, int ext_len);
	BOOL RenameRandomFname(void *org_path, void *rename_path, int dir_len, int fname_len);
	BOOL WriteRandomData(void *path, FileStat *stat, BOOL skip_hardlink);

	BOOL IsSameContents(FileStat *srcStat, FileStat *dstStat);
	BOOL MakeDigest(void *path, VBuf *vbuf, TDigest *digest, BYTE *val, _int64 *fsize=NULL);

	void DstRequest(DstReqKind kind);
	BOOL WaitDstRequest(void);
	BOOL CheckDstRequest(void);
	BOOL ReadDstStat(void);
	BOOL MakeHashTable(void);
	void FreeDstStat(void);
	static int SortStatFunc(const void *stat1, const void *stat2);
	BOOL WriteProc(int dir_len);
	BOOL CaseAlignProc(int dir_len=-1);
	BOOL WriteDirProc(int dir_len);
	BOOL ExecDirQueue(void);
	BOOL SetDirExtData(FileStat *stat);
	DigestCalc *GetDigestCalc(DigestObj *obj, int require_size);
	BOOL PutDigestCalc(DigestCalc *obj, DigestCalc::Status status);
	BOOL MakeDigestAsync(DigestObj *obj);
	BOOL CheckDigests(CheckDigestMode mode);
	BOOL WrieFileWithReduce(HANDLE hFile, void *buf, DWORD size, DWORD *written,
			OVERLAPPED *overwrap);
	BOOL WriteFileProc(int dst_len);
	BOOL WriteFileBackupProc(HANDLE fh, int dst_len);
	BOOL ChangeToWriteModeCore(BOOL is_finish=FALSE);
	BOOL ChangeToWriteMode(BOOL is_finish=FALSE);
	BOOL AllocReqBuf(int req_size, _int64 _data_size, ReqBuf *buf);
	BOOL PrepareReqBuf(int req_size, _int64 data_size, _int64 file_id, ReqBuf *buf);
	BOOL SendRequest(Command command, ReqBuf *buf=NULL, FileStat *stat=NULL);
	BOOL RecvRequest(void);
	void WriteReqDone(void);
	void SetErrFileID(_int64 file_id);
	BOOL SetFinishFileID(_int64 _file_id, MoveObj::Status status);

	BOOL InitSrcPath(int idx);
	BOOL InitDeletePath(int idx);
	BOOL InitDstPath(void);
	FsType GetFsType(const void *root_dir);
	int GetSectorSize(const void *root_dir);
	BOOL IsSameDrive(const void *root1, const void *root2);
	int MakeUnlimitedPath(WCHAR *buf);
	BOOL PutList(void *path, DWORD opt, DWORD lastErr=0, BYTE *digest=NULL);

	inline BOOL IsParentOrSelfDirs(void *name) {
		return *(BYTE *)name == '.' && (!strcmpV(name, DOT_V) || !strcmpV(name, DOTDOT_V));
	}
	int FdatToFileStat(WIN32_FIND_DATAW *fdat, FileStat *stat, BOOL is_usehash);
	Confirm::Result ConfirmErr(const char *message, const void *path=NULL, DWORD flags=0);
	BOOL ConvertExternalPath(const void *path, void *buf, int buf_len);
	BOOL Wait(DWORD tick=0);
};

//	RegisterInfoProc
//	InitConfigProc
//	StartProc


