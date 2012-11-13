#ifndef __SOCKET_H__
#define __SOCKET_H__

// IPv6 capable, non-blocking socket class for use with wxWidgets.
// Error codes are the same as used by the POSIX socket functions,
// see 'man 2 socket', 'man 2 connect', ...

class CSocketEventHandler;
class CSocketEventSource;
class CSocketEvent
{
public:
	enum EventType
	{
		hostaddress,

		// This is a nonfatal condition. It
		// means there are additional addresses to try.
		connection_next,
		connection,
		read,
		write,
		close
	};

	CSocketEvent(CSocketEventHandler* pSocketEventHandler, CSocketEventSource* pSource, enum EventType type, const wxChar* data);
	CSocketEvent(CSocketEventHandler* pSocketEventHandler, CSocketEventSource* pSource, enum EventType type, int error = 0);
	~CSocketEvent();

	CSocketEventSource* GetSocketEventSource() const { return m_pSource; }
	enum EventType GetType() const { return m_type; }
	CSocketEventHandler* GetSocketEventHandler() const { return m_pSocketEventHandler; }

	wxString GetData() const;
	int GetError() const { return m_error; }

protected:
	CSocketEventSource* m_pSource;
	const enum EventType m_type;
	wxChar *m_data;
	int m_error;
	CSocketEventHandler* m_pSocketEventHandler;

	friend class CSocketEventDispatcher;
};

class CSocketEventDispatcher : protected wxEvtHandler
{
public:
	void SendEvent(CSocketEvent* evt);
	void RemovePending(const CSocketEventHandler* pHandler);
	void RemovePending(const CSocketEventSource* pSource);
	void UpdatePending(const CSocketEventHandler* pOldHandler, const CSocketEventSource* pOldSource, CSocketEventHandler* pNewHandler, CSocketEventSource* pNewSource);

	static CSocketEventDispatcher& Get();

private:
	CSocketEventDispatcher();
	~CSocketEventDispatcher();

	virtual bool ProcessEvent(wxEvent& event);

	std::list<CSocketEvent*> m_pending_events;

	wxCriticalSection m_sync;

	static CSocketEventDispatcher m_dispatcher;

	bool m_inside_loop;
};

class CSocketEventHandler
{
public:
	CSocketEventHandler() {};
	virtual ~CSocketEventHandler();

	virtual void OnSocketEvent(CSocketEvent& event) = 0;
};

class CSocketEventSource
{
public:
	virtual ~CSocketEventSource();
};

class CSocketThread;
class CSocket : public CSocketEventSource
{
	friend class CSocketThread;
public:
	CSocket(CSocketEventHandler* pEvtHandler);
	virtual ~CSocket();

	enum SocketState
	{
		// How the socket is initially
		none,

		// Only in listening and connecting states you can get a connection event.
		// After sending the event, socket is in connected state
		listening,
		connecting,

		// Only in this state you can get send or receive events
		connected,

		// Graceful shutdown, you get close event once done
		closing,
		closed
	};
	enum SocketState GetState();

	enum address_family
	{
		unspec, // AF_UNSPEC
		ipv4,   // AF_INET
		ipv6    // AF_INET6
	};

	// Connects to the given host, given as name, IPv4 or IPv6 address.
	// Returns 0 on success, else an error code. Note: EINPROGRESS is
	// not really an error. On success, you should still wait for the
	// connection event.
	// If host is a name that can be resolved, a hostaddress socket event gets sent.
	// Once connections got established, a connection event gets sent. If
	// connection could not be established, a close event gets sent.
	int Connect(wxString host, unsigned int port, enum address_family family = unspec);

	// After receiving a send or receive event, you can call these functions
	// as long as their return value is positive.
	int Read(void *buffer, unsigned int size, int& error);
	int Peek(void *buffer, unsigned int size, int& error);
	int Write(const void *buffer, unsigned int size, int& error);

	int Close();

	// Returns empty string on error
	wxString GetLocalIP(bool strip_zone_index = false) const;
	wxString GetPeerIP(bool strip_zone_index = false) const;

	// -1 on error
	int GetLocalPort(int& error);
	int GetRemotePort(int& error);

	// If connected, either ipv4 or ipv6, unspec otherwise
	enum address_family GetAddressFamily() const;

	static wxString GetErrorString(int error);
	static wxString GetErrorDescription(int error);

	// Can only be called if the state is none
	void SetEventHandler(CSocketEventHandler* pEvtHandler);
	CSocketEventHandler* GetEventHandler() { return m_pEvtHandler; }

	static bool Cleanup(bool force);

	static wxString AddressToString(const struct sockaddr* addr, int addr_len, bool with_port = true, bool strip_zone_index = false);

	int Listen(enum address_family family, int port = 0);
	CSocket* Accept(int& error);

	enum Flags
	{
		flag_nodelay = 0x01,
		flag_keepalive = 0x02
	};

	void SetFlags(int flags);

	// If called on listen socket, sizes will be inherited by
	// accepted sockets
	void SetBufferSizes(int size_read, int size_write);

protected:
	static int DoSetFlags(int fd, int flags, int flags_mask);
	static int DoSetBufferSizes(int fd, int size_read, int size_write);
	static int SetNonblocking(int fd);

	void DetachThread();

	CSocketEventHandler* m_pEvtHandler;

	int m_fd;

	enum SocketState m_state;

	CSocketThread* m_pSocketThread;

	wxString m_host;
	unsigned int m_port;
	int m_family;

	int m_flags;

	int m_buffer_sizes[2];
};

#ifdef __WXMSW__

#ifndef EISCONN
#define EISCONN WSAEISCONN
#endif
#ifndef EINPROGRESS
#define EINPROGRESS WSAEINPROGRESS
#endif
#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#endif
#ifndef EADDRINUSE
#define EADDRINUSE WSAEADDRINUSE
#endif
#ifndef ENOBUFS
#define ENOBUFS WSAENOBUFS
#endif
#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT
#endif
#ifndef EALREADY
#define EALREADY WSAEALREADY
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED WSAECONNREFUSED
#endif
#ifndef ENOTSOCK
#define ENOTSOCK WSAENOTSOCK
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT WSAETIMEDOUT
#endif
#ifndef ENETUNREACH
#define ENETUNREACH WSAENETUNREACH
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH WSAEHOSTUNREACH
#endif
#ifndef ENOTCONN
#define ENOTCONN WSAENOTCONN
#endif
#ifndef ENETRESET
#define ENETRESET WSAENETRESET
#endif
#ifndef EOPNOTSUPP
#define EOPNOTSUPP WSAEOPNOTSUPP
#endif
#ifndef ESHUTDOWN
#define ESHUTDOWN WSAESHUTDOWN
#endif
#ifndef EMSGSIZE
#define EMSGSIZE WSAEMSGSIZE
#endif
#ifndef ECONNABORTED
#define ECONNABORTED WSAECONNABORTED
#endif
#ifndef ECONNRESET
#define ECONNRESET WSAECONNRESET
#endif

// For the future:
// Handle ERROR_NETNAME_DELETED=64
#endif //__WXMSW__

#endif //__SOCKET_H__
