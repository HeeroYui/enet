/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/debug.hpp>
#include <enet/Tcp.hpp>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <etk/stdTools.hpp>

#ifdef __TARGET_OS__Windows
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <netdb.h>
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
	enet::Tcp::Tcp(SOCKET _idSocket, const std::string& _name) :
#else
	enet::Tcp::Tcp(int32_t _idSocket, const std::string& _name) :
#endif
  m_socketId(_idSocket),
  m_name(_name),
  m_status(status::link) {
	//Initialize the pollfd structure
	memset(&m_fds[0], 0 , sizeof(m_fds));
	//Set up the initial listening socket
	#ifndef __TARGET_OS__Windows
		m_fds[0].fd = _idSocket;
		m_fds[0].events = POLLIN | POLLERR;
	#endif
}

enet::Tcp::Tcp(Tcp&& _obj) :
  m_socketId(_obj.m_socketId),
  m_name(_obj.m_name),
  m_status(_obj.m_status) {
	#ifdef __TARGET_OS__Windows
		_obj.m_socketId = INVALID_SOCKET;
	#else
		_obj.m_socketId = -1;
	#endif
	_obj.m_name = "";
	_obj.m_status = status::error;
	#ifndef __TARGET_OS__Windows
		m_fds[0] = _obj.m_fds[0];
		memset(&m_fds[0], 0 , sizeof(m_fds));
	#endif
}

enet::Tcp::~Tcp() {
	unlink();
}

enet::Tcp& enet::Tcp::operator = (enet::Tcp&& _obj) {
	unlink();
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
	#ifndef __TARGET_OS__Windows
		m_fds[0] = _obj.m_fds[0];
		memset(&m_fds[0], 0 , sizeof(m_fds));
	#endif
	return *this;
}

bool enet::Tcp::unlink() {
	if (m_socketId >= 0) {
		ENET_INFO("Close socket (start)");
		#ifdef __TARGET_OS__Windows
			shutdown(m_socketId, SD_BOTH);
		#else
			shutdown(m_socketId, SHUT_RDWR);
		#endif
		#ifdef __TARGET_OS__Windows
			closesocket(m_socketId);
		#else
			close(m_socketId);
		#endif
		ENET_INFO("Close socket (done)");
		m_socketId = -1;
	}
	m_status = status::unlink;
	return true;
}


int32_t enet::Tcp::read(void* _data, int32_t _maxLen) {
	if (m_status != status::link) {
		ENET_ERROR("Can not read on unlink connection");
		return -1;
	}
	int32_t size = -1;
	// Initialize the timeout to 3 minutes. If no activity after 3 minutes this program will end. timeout value is based on milliseconds.
	int timeout = (3 * 60 * 1000);
	#ifdef __TARGET_OS__Windows
		
	#else
		int nfds = 1;
		// Call poll() and wait 3 minutes for it to complete.
		int rc = poll(m_fds, nfds, timeout);
		// Check to see if the poll call failed.
		if (rc < 0) {
			ENET_ERROR("	poll() failed");
			return-1;
		}
		// Check to see if the 3 minute time out expired.
		if (rc == 0) {
			ENET_ERROR("	poll() timed out.\n");
			return -2;
		}
		bool closeConn = false;
		// Receive all incoming data on this socket before we loop back and call poll again.
		// Receive data on this connection until the recv fails with EWOULDBLOCK.
		// If any other failure occurs, we will close the connection.
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			//ENET_DEBUG("Read on socketid = " << m_fds[0].fd );
			rc = recv(m_fds[0].fd, _data, _maxLen, 0);
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
	#endif
	return size;
}


int32_t enet::Tcp::write(const void* _data, int32_t _len) {
	if (m_status != status::link) {
		ENET_ERROR("Can not write on unlink connection");
		return -1;
	}
	//ENET_DEBUG("write on socketid = " << m_socketId << " data@=" << int64_t(_data) << " size=" << _len );
	int32_t size;
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		#ifdef __TARGET_OS__Windows
			size = ::send(m_socketId, (const char *)_data, _len, 0);
		#else
			size = ::write(m_socketId, _data, _len);
		#endif
	}
	if (    size != _len
	     && errno != 0) {
		ENET_ERROR("PB when writing data on the FD : request=" << _len << " have=" << size << ", erno=" << errno << "," << strerror(errno));
		m_status = status::error;
		return -1;
	}
	return size;
}
