/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/debug.hpp>
#include <enet/Tcp.hpp>
#include <enet/TcpServer.hpp>
#include <enet/enet.hpp>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <etk/stdTools.hpp>

#ifdef __TARGET_OS__Windows
	#include <winsock2.h>
	#include <ws2tcpip.h>
	//https://msdn.microsoft.com/fr-fr/library/windows/desktop/ms737889(v=vs.85).aspx
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <netdb.h>
#endif

enet::TcpServer::TcpServer() :
  m_socketId(-1),
  m_host("127.0.0.1"),
  m_port(23191) {
	
}

enet::TcpServer::~TcpServer() {
	unlink();
}

void enet::TcpServer::setIpV4(uint8_t _fist, uint8_t _second, uint8_t _third, uint8_t _quatro) {
	std::string tmpname;
	tmpname  = etk::to_string(_fist);
	tmpname += ".";
	tmpname += etk::to_string(_second);
	tmpname += ".";
	tmpname += etk::to_string(_third);
	tmpname += ".";
	tmpname += etk::to_string(_quatro);
	setHostNane(tmpname);
}

void enet::TcpServer::setHostNane(const std::string& _name) {
	if (_name == m_host) {
		return;
	}
	m_host = _name;
}

void enet::TcpServer::setPort(uint16_t _port) {
	if (_port == m_port) {
		return;
	}
	m_port = _port;
}

#ifdef __TARGET_OS__Windows
	bool enet::TcpServer::link() {
		if (enet::isInit() == false) {
			ENET_ERROR("Need call enet::init(...) before accessing to the socket");
			return false;
		}
		ENET_INFO("Start connection on " << m_host << ":" << m_port);
		
		struct addrinfo *result = nullptr;
		struct addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;
		
		// Resolve the server address and port
		std::string portValue = etk::to_string(m_port);
		int iResult = getaddrinfo(nullptr, portValue.c_str(), &hints, &result);
		if (iResult != 0) {
			ENET_ERROR("getaddrinfo failed with error: " << iResult);
			return 1;
		}
		
		// open in Socket normal mode
		m_socketId = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (m_socketId != INVALID_SOCKET) {
			ENET_ERROR("ERROR while opening socket : errno=" << errno << "," << strerror(errno));
			freeaddrinfo(result);
			return false;
		}
		// set the reuse of the socket if previously opened :
		int sockOpt = 1;
		if(setsockopt(m_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&sockOpt, sizeof(int)) != 0) {
			ENET_ERROR("ERROR while configuring socket re-use : errno=" << errno << "," << strerror(errno));
			return false;
		}
		ENET_INFO("Start binding Socket ... (can take some time ...)");
		if (bind(m_socketId, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
			ENET_ERROR("ERROR on binding errno=" << WSAGetLastError());
			freeaddrinfo(result);
			closesocket(m_socketId);
			m_socketId = INVALID_SOCKET;
			return false;
		}
		return true;
	}
#else
	bool enet::TcpServer::link() {
		if (enet::isInit() == false) {
			ENET_ERROR("Need call enet::init(...) before accessing to the socket");
			return false;
		}
		ENET_INFO("Start connection on " << m_host << ":" << m_port);
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
			#ifdef __TARGET_OS__Windows
				closesocket(m_socketId);
				m_socketId = INVALID_SOCKET;
			#else
				close(m_socketId);
				m_socketId = -1;
			#endif
			return false;
		}
		return true;
	}
#endif

enet::Tcp enet::TcpServer::waitNext() {
	if (enet::isInit() == false) {
		ENET_ERROR("Need call enet::init(...) before accessing to the socket");
		return std::move(enet::Tcp());
	}
	ENET_INFO("End binding Socket ... (start listen)");
	#ifdef __TARGET_OS__Windows
		int ret = listen(m_socketId, SOMAXCONN);
		if (ret == SOCKET_ERROR) {
			ENET_ERROR("listen failed with error: " << WSAGetLastError());
			return enet::Tcp();;
		}
	#else
		listen(m_socketId, 1); // 1 is for the number of connection at the same time ...
	#endif
	ENET_INFO("End listen Socket ... (start accept)");
	struct sockaddr_in clientAddr;
	socklen_t clilen = sizeof(clientAddr);
	int32_t socketIdClient = accept(m_socketId, (struct sockaddr *) &clientAddr, &clilen);
	if (socketIdClient < 0) {
		ENET_ERROR("ERROR on accept errno=" << errno << "," << strerror(errno));
		#ifdef __TARGET_OS__Windows
			closesocket(m_socketId);
			m_socketId = INVALID_SOCKET;
		#else
			close(m_socketId);
			m_socketId = -1;
		#endif
		
		return enet::Tcp();
	}
	ENET_INFO("End configuring Socket ... Find New one");
	return enet::Tcp(socketIdClient, m_host + ":" + etk::to_string(m_port));
}


bool enet::TcpServer::unlink() {
	#ifdef __TARGET_OS__Windows
		if (m_socketId != INVALID_SOCKET) {
			ENET_INFO(" close server socket");
			closesocket(m_socketId);
			m_socketId = INVALID_SOCKET;
		}
	#else
		if (m_socketId >= 0) {
			ENET_INFO(" close server socket");
			close(m_socketId);
			m_socketId = -1;
		}
	#endif
	return true;
}
