/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/debug.h>
#include <enet/Tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <etk/stdTools.h>
#include <netinet/tcp.h>

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
  m_socketId(-1),
  m_name(),
  m_status(status::error) {
	
}

enet::Tcp::Tcp(int32_t _idSocket, const std::string& _name) :
  m_socketId(_idSocket),
  m_name(_name),
  m_status(status::link) {
	#if 1
		//Initialize the pollfd structure
		memset(m_fds, 0 , sizeof(m_fds));
		//Set up the initial listening socket
		m_fds[0].fd = _idSocket;
		m_fds[0].events = POLLIN | POLLERR;
	#endif
}

enet::Tcp::Tcp(Tcp&& _obj) :
  m_socketId(_obj.m_socketId),
  m_name(_obj.m_name),
  m_status(_obj.m_status) {
	_obj.m_socketId = -1;
	_obj.m_name = "";
	_obj.m_status = status::error;
	m_fds[0] = _obj.m_fds[0];
	#if 1
		memset(_obj.m_fds, 0 , sizeof(_obj.m_fds));
	#endif
}
enet::Tcp::~Tcp() {
	unlink();
}

enet::Tcp& enet::Tcp::operator = (enet::Tcp&& _obj) {
	unlink();
	m_socketId = _obj.m_socketId;
	_obj.m_socketId = -1;
	m_name = _obj.m_name;
	_obj.m_name = "";
	m_status = _obj.m_status;
	_obj.m_status = status::error;
	m_fds[0] = _obj.m_fds[0];
	#if 1
		memset(_obj.m_fds, 0 , sizeof(_obj.m_fds));
	#endif
	return *this;
}

bool enet::Tcp::unlink() {
	if (m_socketId >= 0) {
		ENET_INFO("Close socket (start)");
		shutdown(m_socketId, SHUT_RDWR);
		close(m_socketId);
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
	int nfds = 1;
	// Initialize the timeout to 3 minutes. If no activity after 3 minutes this program will end. timeout value is based on milliseconds.
	int timeout = (3 * 60 * 1000);
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
	rc = recv(m_fds[0].fd, _data, _maxLen, 0);
	if (rc < 0) {
		if (errno != EWOULDBLOCK) {
			ENET_ERROR("	recv() failed");
			closeConn = true;
		}
	}
	// Check to see if the connection has been closed by the client
	if (rc == 0) {
		ENET_INFO("	Connection closed");
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
	return size;
}


int32_t enet::Tcp::write(const void* _data, int32_t _len) {
	if (m_status != status::link) {
		ENET_ERROR("Can not write on unlink connection");
		return -1;
	}
	int32_t size = ::write(m_socketId, _data, _len);
	if (    size != _len
	     && errno != 0) {
		ENET_ERROR("PB when writing data on the FD : request=" << _len << " have=" << size << ", erno=" << errno << "," << strerror(errno));
		m_status = status::error;
		return -1;
	}
	return size;
}
