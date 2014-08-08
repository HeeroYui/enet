/**
 * @author Edouard DUPIN
 * 
 * @copyright 2014, Edouard DUPIN, all right reserved
 * 
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

#ifdef __class__
	#undef  __class__
#endif
#define __class__ ("Tcp")

enet::Tcp::Tcp() :
  m_socketId(-1),
  m_socketIdClient(-1),
  m_host("127.0.0.1"),
  m_port(23191),
  m_server(false),
  m_status(statusUnlink) {
	
}

enet::Tcp::~Tcp() {
	unlink();
}

void enet::Tcp::setIpV4(uint8_t _fist, uint8_t _second, uint8_t _third, uint8_t _quatro) {
	std::string tmpname;
	tmpname  = std::to_string(_fist);
	tmpname += ".";
	tmpname += std::to_string(_second);
	tmpname += ".";
	tmpname += std::to_string(_third);
	tmpname += ".";
	tmpname += std::to_string(_quatro);
	setHostNane(tmpname);
}

void enet::Tcp::setHostNane(const std::string& _name) {
	if (_name == m_host) {
		return;
	}
	if (m_status == statusLink) {
		ENET_ERROR("Can not change parameter while connection is started");
		return;
	}
	m_host = _name;
}

void enet::Tcp::setPort(uint16_t _port) {
	if (_port == m_port) {
		return;
	}
	if (m_status == statusLink) {
		ENET_ERROR("Can not change parameter while connection is started");
		return;
	}
	m_port = _port;
}

void enet::Tcp::setServer(bool _status) {
	if (_status == m_server) {
		return;
	}
	if (m_status == statusLink) {
		ENET_ERROR("Can not change parameter while connection is started");
		return;
	}
	m_server = _status;
}

bool enet::Tcp::link() {
	if (m_status == statusLink) {
		ENET_ERROR("Connection is already started");
		return false;
	}
	ENET_INFO("Start connection on " << m_host << ":" << m_port);
	if (m_server == false) {
		#define MAX_TEST_TIME  (5)
		for(int32_t iii=0; iii<MAX_TEST_TIME ;iii++) {
			// open in Socket normal mode
			m_socketIdClient = socket(AF_INET, SOCK_STREAM, 0);
			if (m_socketIdClient < 0) {
				ENET_ERROR("ERROR while opening socket : errno=" << errno << "," << strerror(errno));
				usleep(200000);
				continue;
			}
			ENET_INFO("Try connect on socket ... (" << iii+1 << "/" << MAX_TEST_TIME << ")");
			struct sockaddr_in servAddr;
			struct hostent* server = gethostbyname(m_host.c_str());
			if (server == NULL) {
				ENET_ERROR("ERROR, no such host : " << m_host);
				usleep(200000);
				continue;
			}
			bzero((char *) &servAddr, sizeof(servAddr));
			servAddr.sin_family = AF_INET;
			bcopy((char *)server->h_addr, (char *)&servAddr.sin_addr.s_addr, server->h_length);
			servAddr.sin_port = htons(m_port);
			ENET_INFO("Start connexion ...");
			if (connect(m_socketIdClient,(struct sockaddr *) &servAddr,sizeof(servAddr)) != 0) {
				if(errno != EINPROGRESS) {
					if(errno != ENOENT && errno != EAGAIN && errno != ECONNREFUSED) {
						ENET_ERROR("ERROR connecting on : errno=" << errno << "," << strerror(errno));
					}
					close(m_socketIdClient);
					m_socketIdClient = -1;
				}
				ENET_ERROR("ERROR connecting, maybe retry ... errno=" << errno << "," << strerror(errno));
				usleep(500000);
				continue;
			}
			// if we are here ==> then the connextion is done corectly ...
			break;
		}
		if (m_socketIdClient<0) {
			ENET_ERROR("ERROR connecting ... (after all try)");
			return false;
		} else {
			m_status = statusLink;
			ENET_DEBUG("Connection done");
		}
	} else {
		// open in Socket normal mode
		m_socketId = socket(AF_INET, SOCK_STREAM, 0);
		if (m_socketId < 0) {
			ENET_ERROR("ERROR while opening socket : errno=" << errno << "," << strerror(errno));
			return false;
		}
		// set the reuse of the socket if previously opened :
		int sockOpt = 1;
		if(setsockopt(m_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&sockOpt, sizeof(int)) != 0) {
			ENET_ERROR("ERROR while configuring socket re-use : errno=" << errno << "," << strerror(errno));
			return false;
		}
		// clear all
		struct sockaddr_in servAddr;
		bzero((char *) &servAddr, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = INADDR_ANY;
		servAddr.sin_port = htons(m_port);
		ENET_INFO("Start binding Socket ... (can take some time ...)");
		if (bind(m_socketId, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
			ENET_ERROR("ERROR on binding errno=" << errno << "," << strerror(errno));
			close(m_socketId);
			m_socketId = -1;
			return false;
		}
		ENET_INFO("End binding Socket ... (start listen)");
		listen(m_socketId,1); // 1 is for the number of connection at the same time ...
		ENET_INFO("End listen Socket ... (start accept)");
		struct sockaddr_in clientAddr;
		socklen_t clilen = sizeof(clientAddr);
		m_socketIdClient = accept(m_socketId, (struct sockaddr *) &clientAddr, &clilen);
		if (m_socketIdClient < 0) {
			ENET_ERROR("ERROR on accept errno=" << errno << "," << strerror(errno));
			close(m_socketId);
			m_socketId = -1;
			return false;
		} else {
			m_status = statusLink;
			ENET_DEBUG("Connection done");
		}
	}
	ENET_INFO("End configuring Socket ...");
	return true;
}


bool enet::Tcp::unlink() {
	if (m_socketIdClient >= 0) {
		ENET_INFO(" close client socket");
		close(m_socketIdClient);
		m_socketIdClient = -1;
	}
	if (m_socketId >= 0) {
		ENET_INFO(" close server socket");
		close(m_socketId);
		m_socketId = -1;
	}
	m_status = statusUnlink;
	return true;
}


int32_t enet::Tcp::read(void* _data, int32_t _maxLen) {
	if (m_status != statusLink) {
		ENET_ERROR("Can not read on unlink connection");
		return -1;
	}
	int32_t size = ::read(m_socketIdClient, _data, _maxLen);
	if (    size != _maxLen
	     && errno != 0) {
		ENET_ERROR("PB when reading data on the FD : request=" << _maxLen << " have=" << size << ", erno=" << errno << "," << strerror(errno));
		m_status = statusError;
		return -1;
	}
	return size;
}

int32_t enet::Tcp::write(const void* _data, int32_t _len) {
	if (m_status != statusLink) {
		ENET_ERROR("Can not write on unlink connection");
		return -1;
	}
	int32_t size = ::write(m_socketIdClient, _data, _len);
	if (    size != _len
	     && errno != 0) {
		ENET_ERROR("PB when writing data on the FD : request=" << _len << " have=" << size << ", erno=" << errno << "," << strerror(errno));
		m_status = statusError;
		return -1;
	}
	return size;
}

