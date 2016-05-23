/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/debug.h>
#include <enet/Tcp.h>
#include <enet/TcpServer.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <etk/stdTools.h>


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

bool enet::TcpServer::link() {
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
		close(m_socketId);
		m_socketId = -1;
		return false;
	}
	return true;
}

enet::Tcp enet::TcpServer::waitNext() {
	ENET_INFO("End binding Socket ... (start listen)");
	listen(m_socketId,1); // 1 is for the number of connection at the same time ...
	ENET_INFO("End listen Socket ... (start accept)");
	struct sockaddr_in clientAddr;
	socklen_t clilen = sizeof(clientAddr);
	int32_t socketIdClient = accept(m_socketId, (struct sockaddr *) &clientAddr, &clilen);
	if (socketIdClient < 0) {
		ENET_ERROR("ERROR on accept errno=" << errno << "," << strerror(errno));
		close(m_socketId);
		m_socketId = -1;
		return enet::Tcp();
	}
	ENET_INFO("End configuring Socket ... Find New one");
	return enet::Tcp(socketIdClient, m_host + ":" + etk::to_string(m_port));
}


bool enet::TcpServer::unlink() {
	if (m_socketId >= 0) {
		ENET_INFO(" close server socket");
		close(m_socketId);
		m_socketId = -1;
	}
	return true;
}
