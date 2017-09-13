/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <enet/debug.hpp>
#include <enet/Tcp.hpp>
#include <sys/types.h>
extern "C" {
	#include <errno.h>
	#include <unistd.h>
	#include <string.h>
}
#include <etk/stdTools.hpp>
#include <ethread/tools.hpp>

#ifdef __TARGET_OS__Windows
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <netdb.h>
#endif

#ifdef ENET_STORE_INPUT
	static uint32_t baseID = 0;
#endif

bool enet::Tcp::setTCPNoDelay(bool _enabled) {
	if (m_socketId >= 0) {
		int flag = _enabled==true?1:0;
		if(setsockopt(m_socketId, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) == -1) {
			return false;
		}
		return true;
	}
	return false;
}


enet::Tcp::Tcp() :
#ifdef __TARGET_OS__Windows
  m_socketId(INVALID_SOCKET),
#else
  m_socketId(-1),
#endif
  m_name(),
  m_status(status::error) {
	
}

#ifdef __TARGET_OS__Windows
	enet::Tcp::Tcp(SOCKET _idSocket, const etk::String& _name) :
#else
	enet::Tcp::Tcp(int32_t _idSocket, const etk::String& _name) :
#endif
  m_socketId(_idSocket),
  m_name(_name),
  m_status(status::link) {
	#ifdef ENET_STORE_INPUT
		m_nodeStoreInput = etk::FSNode("CACHE:StoreTCPdata_" + etk::toString(baseID++) + ".tcp");
		m_nodeStoreInput.fileOpenWrite();
	#endif
}

enet::Tcp::Tcp(Tcp&& _obj) :
  m_socketId(_obj.m_socketId),
  m_name(_obj.m_name),
  m_status(_obj.m_status) {
	#ifdef ENET_STORE_INPUT
		m_nodeStoreInput = etk::FSNode("CACHE:StoreTCPdata_" + etk::toString(baseID++) + ".tcp");
		m_nodeStoreInput.fileOpenWrite();
	#endif
	#ifdef __TARGET_OS__Windows
		_obj.m_socketId = INVALID_SOCKET;
	#else
		_obj.m_socketId = -1;
	#endif
	_obj.m_name = "";
	_obj.m_status = status::error;
}

enet::Tcp::~Tcp() {
	unlink();
}

enet::Tcp& enet::Tcp::operator = (enet::Tcp&& _obj) {
	unlink();
	#ifdef ENET_STORE_INPUT
		m_nodeStoreInput = etk::FSNode("CACHE:StoreTCPdata_" + etk::toString(baseID++) + ".tcp");
		m_nodeStoreInput.fileOpenWrite();
	#endif
	m_socketId = _obj.m_socketId;
	#ifdef __TARGET_OS__Windows
		_obj.m_socketId = INVALID_SOCKET;
	#else
		_obj.m_socketId = -1;
	#endif
	m_name = _obj.m_name;
	_obj.m_name = "";
	m_status = _obj.m_status;
	_obj.m_status = status::error;
	return *this;
}

bool enet::Tcp::unlink() {
	// prevent call while stoping ...
	m_status = status::unlink;
	if (m_socketId >= 0) {
		ENET_INFO("Close socket (start)");
		#ifdef __TARGET_OS__Windows
			shutdown(m_socketId, SD_BOTH);
			// Release hand of the socket to permit the Select to exit ... ==> otherwise it lock ...
			ethread::sleepMilliSeconds((20));
			closesocket(m_socketId);
			m_socketId = INVALID_SOCKET;
		#else
			shutdown(m_socketId, SHUT_RDWR);
			// Release hand of the socket to permit the Select to exit ... ==> otherwise it lock ...
			ethread::sleepMilliSeconds((20));
			close(m_socketId);
			m_socketId = -1;
		#endif
		ENET_INFO("Close socket (done)");
	}
	return true;
}


int32_t enet::Tcp::read(void* _data, int32_t _maxLen) {
	if (m_status != status::link) {
		ENET_ERROR("Can not read on unlink connection");
		return -1;
	}
	int32_t size = -1;
	
	fd_set sock;
	// Initialize the timeout to 3 minutes. If no activity after 3 minutes this program will end. timeout value is based on milliseconds.
	struct timeval timeOutStruct;
	timeOutStruct.tv_sec = (3 * 60 * 1000);
	timeOutStruct.tv_usec = 0;
	FD_ZERO(&sock);
	FD_SET(m_socketId,&sock);
	ENET_VERBOSE("	select ...");
	int rc = select(m_socketId+1, &sock, NULL, NULL, &timeOutStruct);
	ENET_VERBOSE("	select (done)");
	// Check to see if the poll call failed.
	if (rc < 0) {
		ENET_ERROR("	select() failed");
		return -1;
	}
	// Check to see if the 3 minute time out expired.
	if (rc == 0) {
		ENET_ERROR("	select() timed out.");
		return -2;
	}
	if (!FD_ISSET(m_socketId, &sock)) {
		ENET_ERROR("	select() id is not set...");
		return -1;
	}
	bool closeConn = false;
	// Receive all incoming data on this socket before we loop back and call poll again.
	// Receive data on this connection until the recv fails with EWOULDBLOCK.
	// If any other failure occurs, we will close the connection.
	{
		ethread::UniqueLock lock(m_mutex);
		rc = recv(m_socketId, (char *)_data, _maxLen, 0);
	}
	if (rc < 0) {
		if (errno != EWOULDBLOCK) {
			ENET_ERROR("	recv() failed");
			closeConn = true;
		}
	}
	// Check to see if the connection has been closed by the client
	if (rc == 0) {
		ENET_INFO("Connection closed");
		closeConn = true;
		size = 0;
	}
	if (closeConn == false) {
		// Data was received
		size = rc;
	} else {
		// If the close_conn flag was turned on, we need to clean up this active connection.
		// This clean up process includes removing the descriptor.
		ENET_DEBUG("	Set status at remote close ...");
		m_status = status::linkRemoteClose;
	}
	#ifdef ENET_STORE_INPUT
		m_nodeStoreInput.fileWrite(_data, 1, size);
	#endif
	return size;
}


int32_t enet::Tcp::write(const void* _data, int32_t _len) {
	if (m_status != status::link) {
		ENET_ERROR("Can not write on unlink connection");
		return -1;
	}
	if (_data == nullptr) {
		ENET_ERROR("try write nullptr data on TCP socket");
		return -1;
	}
	if (_len <= 0) {
		if (_len == 0) {
			ENET_WARNING("try write data with lenght=" << _len << " ==> bad case");
			return 0;
		}
		ENET_ERROR("try write data with lenght=" << _len << " ==> bad case");
		elog::displayBacktrace();
		return -1;
	}
	//ENET_DEBUG("write on socketid = " << m_socketId << " data@=" << int64_t(_data) << " size=" << _len );
	int32_t size;
	{
		ethread::UniqueLock lock(m_mutex);
		size = ::send(m_socketId, (const char *)_data, _len, 0);
	}
	if (    size != _len
	     && errno != 0) {
		ENET_ERROR("PB when writing data on the FD : request=" << _len << " have=" << size << ", erno=" << errno << "," << strerror(errno));
		m_status = status::error;
		return -1;
	}
	return size;
}
