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
  m_interface(),
  m_observer(nullptr) {
	if (_isServer == true) {
		ememory::SharedPtr<enet::HttpServer> interface = std::make_shared<enet::HttpServer>(std::move(_connection));
		interface->connectHeader(this, &enet::WebSocket::onReceiveRequest);
		m_interface = interface;
	} else {
		ememory::SharedPtr<enet::HttpClient> interface = std::make_shared<enet::HttpClient>(std::move(_connection));
		interface->connectHeader(this, &enet::WebSocket::onReceiveAnswer);
		m_interface = interface;
	}
	m_interface->connect(this, &enet::WebSocket::onReceiveData);
}

enet::WebSocket::~WebSocket() {
	if (m_interface == nullptr) {
		return;
	}
	stop(true);
}


void enet::WebSocket::start(const std::string& _uri) {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	m_interface->start();
	if (m_interface->isServer() == false) {
		enet::HttpRequest req(enet::HTTPReqType::GET);
		req.setUri(_uri);
		std::dynamic_pointer_cast<enet::HttpClient>(m_interface)->setHeader(req);
	}
}

void enet::WebSocket::stop(bool _inThread) {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	m_interface->stop(_inThread);
	m_interface.reset();
}

void enet::WebSocket::onReceiveData(std::vector<uint8_t>& _data) {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	ENET_ERROR("manage receive data event ...");
}

void enet::WebSocket::onReceiveRequest(const enet::HttpRequest& _data) {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	_data.display();
}

void enet::WebSocket::onReceiveAnswer(const enet::HttpAnswer& _data) {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	_data.display();
}

int32_t enet::WebSocket::write(const void* _data, int32_t _len) {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return -1;
	}
	// TODO : ...
	return -1;
}