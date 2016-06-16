/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/debug.h>
#include <enet/WebSocket.h>
#include <map>
#include <etk/stdTools.h>
#include <string.h>



enet::WebSocket::WebSocket(enet::Tcp _connection, bool _isServer) :
  m_interface(std::move(_connection), _isServer) {
	
}

enet::WebSocket::~WebSocket() {
	
}


void enet::WebSocket::start(const std::string& _uri) {
	if (m_interface.isServer() == true) {
		
	}
	m_interface.start();
	if (m_interface.isServer() == false) {
		m_interface.get(
	}
}

void enet::WebSocket::stop(bool _inThread=false) {
	
}

