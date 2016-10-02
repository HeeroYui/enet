/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/debug.hpp>
#include <enet/Tcp.hpp>
#include <enet/TcpClient.hpp>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <etk/stdTools.hpp>

#ifdef __TARGET_OS__Windows

#else
	#include <sys/socket.h>
#endif

enet::Tcp enet::connectTcpClient(uint8_t _ip1, uint8_t _ip2, uint8_t _ip3, uint8_t _ip4, uint16_t _port, uint32_t _numberRetry) {
	std::string tmpname;
	tmpname  = etk::to_string(_ip1);
	tmpname += ".";
	tmpname += etk::to_string(_ip2);
	tmpname += ".";
	tmpname += etk::to_string(_ip3);
	tmpname += ".";
	tmpname += etk::to_string(_ip4);
	return std::move(enet::connectTcpClient(tmpname, _port, _numberRetry));
}
enet::Tcp enet::connectTcpClient(const std::string& _hostname, uint16_t _port, uint32_t _numberRetry) {
	int32_t socketId = -1;
	ENET_INFO("Start connection on " << _hostname << ":" << _port);
	for(int32_t iii=0; iii<_numberRetry ;iii++) {
		// open in Socket normal mode
		socketId = socket(AF_INET, SOCK_STREAM, 0);
		if (socketId < 0) {
			ENET_ERROR("ERROR while opening socket : errno=" << errno << "," << strerror(errno));
			usleep(200000);
			continue;
		}
		ENET_INFO("Try connect on socket ... (" << iii+1 << "/" << _numberRetry << ")");
		struct sockaddr_in servAddr;
		struct hostent* server = gethostbyname(_hostname.c_str());
		if (server == nullptr) {
			ENET_ERROR("ERROR, no such host : " << _hostname);
			usleep(200000);
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
			usleep(500000);
			continue;
		}
		// if we are here ==> then the connextion is done corectly ...
		break;
	}
	if (socketId<0) {
		ENET_ERROR("ERROR connecting ... (after all try)");
		return std::move(enet::Tcp());
	}
	ENET_DEBUG("Connection done");
	return std::move(enet::Tcp(socketId, _hostname + ":" + etk::to_string(_port)));
}

