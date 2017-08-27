/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <enet/debug.hpp>
#include <enet/Tcp.hpp>
#include <enet/TcpClient.hpp>
#include <enet/enet.hpp>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <etk/stdTools.hpp>

#ifdef __TARGET_OS__Windows

#else
	#include <netinet/in.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

enet::Tcp enet::connectTcpClient(uint8_t _ip1, uint8_t _ip2, uint8_t _ip3, uint8_t _ip4, uint16_t _port, uint32_t _numberRetry, echrono::Duration _timeOut) {
	etk::String tmpname;
	tmpname  = etk::toString(_ip1);
	tmpname += ".";
	tmpname += etk::toString(_ip2);
	tmpname += ".";
	tmpname += etk::toString(_ip3);
	tmpname += ".";
	tmpname += etk::toString(_ip4);
	return etk::move(enet::connectTcpClient(tmpname, _port, _numberRetry, _timeOut));
}

#ifdef __TARGET_OS__Windows
	enet::Tcp enet::connectTcpClient(const etk::String& _hostname, uint16_t _port, uint32_t _numberRetry, echrono::Duration _timeOut) {
		if (enet::isInit() == false) {
			ENET_ERROR("Need call enet::init(...) before accessing to the socket");
			return etk::move(enet::Tcp());
		}
		if (_hostname == "") {
			ENET_ERROR("get connection wihtout hostname");
			return etk::move(enet::Tcp());
		}
		SOCKET socketId = INVALID_SOCKET;
		ENET_INFO("Start connection on " << _hostname << ":" << _port);
		for(int32_t iii=0; iii<_numberRetry ;iii++) {
			if (iii > 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
			// open in Socket normal mode
			socketId = socket(AF_INET, SOCK_STREAM, 0);
			if (socketId < 0) {
				ENET_ERROR("ERROR while opening socket : errno=" << errno << "," << strerror(errno));
				continue;
			}
			ENET_INFO("Try connect on socket ... (" << iii+1 << "/" << _numberRetry << ")");
			
			struct addrinfo hints;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			
			// Resolve the server address and port
			struct addrinfo* result = nullptr;
			etk::String portValue = etk::toString(_port);
			int iResult = getaddrinfo(_hostname.c_str(), portValue.c_str(), &hints, &result);
			if (iResult != 0) {
				ENET_ERROR("getaddrinfo failed with error: " << iResult);
				continue;
			}
			
			// Attempt to connect to an address until one succeeds
			for(struct addrinfo* ptr=result;
			    ptr != nullptr;
			    ptr=ptr->ai_next) {
				ENET_DEBUG(" find one ...");
				// Create a SOCKET for connecting to server
				socketId = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if (socketId == INVALID_SOCKET) {
					ENET_ERROR("socket failed with error: " << WSAGetLastError());
					break;
				}
				// Connect to server.
				iResult = connect(socketId, ptr->ai_addr, (int)ptr->ai_addrlen);
				if (iResult == SOCKET_ERROR) {
					ENET_ERROR("socket connection failed with error: " << WSAGetLastError());
					closesocket(socketId);
					socketId = INVALID_SOCKET;
					continue;
				}
				break;
			}
			freeaddrinfo(result);
			
			if (socketId == INVALID_SOCKET) {
				ENET_ERROR("Unable to connect to server!");
				continue;
			} else {
				break;
			}
		}
		if (socketId == INVALID_SOCKET) {
			ENET_ERROR("ERROR connecting ... (after all try)");
			return etk::move(enet::Tcp());
		}
		ENET_DEBUG("Connection done");
		return etk::move(enet::Tcp(socketId, _hostname + ":" + etk::toString(_port)));
	}
#else
	#include <sys/socket.h>
	enet::Tcp enet::connectTcpClient(const etk::String& _hostname, uint16_t _port, uint32_t _numberRetry, echrono::Duration _timeOut) {
		if (enet::isInit() == false) {
			ENET_ERROR("Need call enet::init(...) before accessing to the socket");
			return etk::move(enet::Tcp());
		}
		if (_hostname == "") {
			ENET_ERROR("get connection wihtout hostname");
			return etk::move(enet::Tcp());
		}
		int32_t socketId = -1;
		ENET_INFO("Start connection on " << _hostname << ":" << _port);
		for(int32_t iii=0; iii<_numberRetry ;iii++) {
			if (iii > 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
			// open in Socket normal mode
			socketId = socket(AF_INET, SOCK_STREAM, 0);
			if (socketId < 0) {
				ENET_ERROR("ERROR while opening socket : errno=" << errno << "," << strerror(errno));
				continue;
			}
			ENET_INFO("Try connect on socket ... (" << iii+1 << "/" << _numberRetry << ")");
			struct sockaddr_in servAddr;
			struct hostent* server = nullptr;
			if (isalpha(_hostname.c_str()[0])) {        /* host address is a name */
				ENET_INFO("Calling gethostbyname with " << _hostname);
				// TODO : This is deprecated use getaddrinfo like windows ...
				server = gethostbyname(_hostname.c_str());
			} else {
				ENET_INFO("Calling gethostbyaddr with " << _hostname);
				struct in_addr addr;
				addr.s_addr = inet_addr(_hostname.c_str());
				if (addr.s_addr == INADDR_NONE) {
					ENET_ERROR("The IPv4 address entered must be a legal address" << _hostname.c_str());
					return etk::move(enet::Tcp());
				} else {
					// TODO : This is deprecated use getaddrinfo like windows ...
					server = gethostbyaddr((char *) &addr, 4, AF_INET);
				}
			}
			if (server == nullptr) {
				ENET_ERROR("ERROR, no such host : " << _hostname);
				continue;
			}
			bzero((char *) &servAddr, sizeof(servAddr));
			servAddr.sin_family = AF_INET;
			bcopy((char *)server->h_addr, (char *)&servAddr.sin_addr.s_addr, server->h_length);
			servAddr.sin_port = htons(_port);
			ENET_INFO("Start connexion ...");
			if (connect(socketId, (struct sockaddr *)&servAddr,sizeof(servAddr)) != 0) {
				if(errno != EINPROGRESS) {
					if(    errno != ENOENT
					    && errno != EAGAIN
					    && errno != ECONNREFUSED) {
						ENET_ERROR("ERROR connecting on : errno=" << errno << "," << strerror(errno));
					}
					#ifdef __TARGET_OS__Windows
						closesocket(socketId);
					#else
						close(socketId);
					#endif
					socketId = -1;
				}
				ENET_ERROR("ERROR connecting, maybe retry ... errno=" << errno << "," << strerror(errno));
				continue;
			}
			break;
		}
		if (socketId<0) {
			ENET_ERROR("ERROR connecting ... (after all try)");
			return etk::move(enet::Tcp());
		}
		ENET_INFO("Connection done");
		return etk::move(enet::Tcp(socketId, _hostname + ":" + etk::toString(_port)));
	}
#endif

