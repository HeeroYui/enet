/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/debug.hpp>
#include <enet/WebSocket.hpp>
#include <map>
#include <etk/stdTools.hpp>
#include <cstring>
#include <random>
#include <algue/base64.hpp>
#include <algue/sha1.hpp>



namespace enet {
	namespace websocket {
		static const uint32_t FLAG_FIN = 0x80;
		static const uint32_t FLAG_MASK = 0x80;
		static const uint32_t OPCODE_FRAME_TEXT = 0x01;
		static const uint32_t OPCODE_FRAME_BINARY = 0x02;
		static const uint32_t OPCODE_FRAME_CLOSE = 0x08;
		static const uint32_t OPCODE_FRAME_PING = 0x09;
		static const uint32_t OPCODE_FRAME_PONG = 0x0A;
	}
}

enet::WebSocket::WebSocket() :
  m_connectionValidate(false),
  m_interface(nullptr),
  m_observer(nullptr),
  m_observerUriCheck(nullptr) {
	
}

enet::WebSocket::WebSocket(enet::Tcp _connection, bool _isServer) :
  m_connectionValidate(false),
  m_interface(nullptr),
  m_observer(nullptr),
  m_observerUriCheck(nullptr) {
	setInterface(std::move(_connection), _isServer);
}

void enet::WebSocket::setInterface(enet::Tcp _connection, bool _isServer) {
	_connection.setTCPNoDelay(true);
	if (_isServer == true) {
		ememory::SharedPtr<enet::HttpServer> interface = ememory::makeShared<enet::HttpServer>(std::move(_connection));
		m_interface = interface;
		if (interface != nullptr) {
			interface->connectHeader(this, &enet::WebSocket::onReceiveRequest);
		}
	} else {
		ememory::SharedPtr<enet::HttpClient> interface = ememory::makeShared<enet::HttpClient>(std::move(_connection));
		m_interface = interface;
		if (interface != nullptr) {
			interface->connectHeader(this, &enet::WebSocket::onReceiveAnswer);
		}
	}
	if (m_interface == nullptr) {
		ENET_ERROR("can not create interface for the websocket");
		return;
	}
	m_interface->connectRaw(this, &enet::WebSocket::onReceiveData);
}

enet::WebSocket::~WebSocket() {
	if (m_interface == nullptr) {
		return;
	}
	stop(true);
}

static std::string generateKey() {
	// create dynamic key:
	std::random_device rd;
	std::mt19937 e2(rd());
	std::uniform_real_distribution<> dist(0, 0xFF);
	uint8_t dataKey[16];
	for (size_t iii=0; iii<16; ++iii) {
		dataKey[iii] = uint8_t(dist(e2));
	}
	return algue::base64::encode(dataKey, 16);
}

static std::string generateCheckKey(const std::string& _key) {
	std::string out = _key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	std::vector<uint8_t> keyData = algue::sha1::encode(out);
	return algue::base64::encode(keyData);
}

void enet::WebSocket::start(const std::string& _uri, const std::vector<std::string>& _listProtocols) {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	m_interface->start();
	if (m_interface->isServer() == false) {
		enet::HttpRequest req(enet::HTTPReqType::HTTP_GET);
		req.setProtocol(enet::HTTPProtocol::http_1_1);
		req.setUri(_uri);
		req.setKey("Upgrade", "websocket");
		req.setKey("Connection", "Upgrade");
		m_checkKey = generateKey();
		req.setKey("Sec-WebSocket-Key", m_checkKey); // this is an example key ...
		m_checkKey = generateCheckKey(m_checkKey);
		req.setKey("Sec-WebSocket-Version", "13");
		req.setKey("Pragma", "no-cache");
		req.setKey("Cache-Control", "no-cache");
		std::string protocolList;
		for (auto &it : _listProtocols) {
			if (it == "") {
				continue;
			}
			if (protocolList != "") {
				protocolList += ", ";
			}
			protocolList += it;
		}
		if (protocolList != "") {
			req.setKey("Sec-WebSocket-Protocol", protocolList);
		}
		ememory::SharedPtr<enet::HttpClient> interface = ememory::dynamicPointerCast<enet::HttpClient>(m_interface);
		if (interface != nullptr) {
			interface->setHeader(req);
			int32_t timeout = 500000; // 5 second
			while (timeout>=0) {
				if (    m_connectionValidate == true
				     || m_interface->isAlive() == false) {
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				timeout--;
			}
			if (    m_connectionValidate == false
			     || m_interface->isAlive() == false) {
				ENET_ERROR("Connection refused by SERVER ...");
			}
		}
	}
}

void enet::WebSocket::stop(bool _inThread) {
	ENET_DEBUG("Stop interface ...");
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	m_interface->stop(_inThread);
	// deadlock ... m_interface.reset();
}

void enet::WebSocket::onReceiveData(enet::Tcp& _connection) {
	uint8_t opcode = 0;
	int32_t len = _connection.read(&opcode, sizeof(uint8_t));
	if (len <= 0) {
		if (len < 0) {
			if (_connection.getConnectionStatus() == enet::Tcp::status::link) {
				ENET_ERROR("Protocol error occured ...");
			}
			ENET_VERBOSE("ReadRaw 1 [STOP]");
			m_interface->stop(true);
			return;
		}
		ENET_ERROR("Time out ... ==> not managed ...");
		ENET_VERBOSE("ReadRaw 2 [STOP]");
		return;
	}
	m_lastReceive = std::chrono::steady_clock::now();
	if ((opcode & 0x80) == 0) {
		ENET_ERROR("Multiple frames ... NOT managed ...");
		m_interface->stop(true);
		return;
	}
	int8_t size1 = 0;
	len = _connection.read(&size1, sizeof(uint8_t));
	if (len <= 0) {
		if (len < 0) {
			if (_connection.getConnectionStatus() == enet::Tcp::status::link) {
				ENET_ERROR("Protocol error occured ...");
			}
			ENET_VERBOSE("ReadRaw 1 [STOP]");
			m_interface->stop(true);
			return;
		}
		ENET_ERROR("Time out ... ==> not managed ...");
		ENET_VERBOSE("ReadRaw 2 [STOP]");
		return;
	}
	uint64_t totalSize = size1 & 0x7F;
	if (totalSize == 126) {
		uint16_t tmpSize;
		len = _connection.read(&tmpSize, sizeof(uint16_t));
		if (len <= 1) {
			if (len < 0) {
				if (_connection.getConnectionStatus() == enet::Tcp::status::link) {
					ENET_ERROR("Protocol error occured ...");
				}
				ENET_VERBOSE("ReadRaw 1 [STOP]");
				m_interface->stop(true);
				return;
			}
			ENET_ERROR("Time out ... ==> not managed ...");
			ENET_VERBOSE("ReadRaw 2 [STOP]");
			return;
		}
		totalSize = tmpSize;
	} else if (totalSize == 127) {
		len = _connection.read(&totalSize, sizeof(uint64_t));
		if (len <= 7) {
			if (len < 0) {
				if (_connection.getConnectionStatus() == enet::Tcp::status::link) {
					ENET_ERROR("Protocol error occured ...");
				}
				ENET_VERBOSE("ReadRaw 1 [STOP]");
				m_interface->stop(true);
				return;
			}
			ENET_ERROR("Time out ... ==> not managed ...");
			ENET_VERBOSE("ReadRaw 2 [STOP]");
			return;
		}
	}
	uint8_t dataMask[4];
	// Need get the mask:
	if ((size1 & 0x80) != 0) {
		len = _connection.read(&dataMask, sizeof(uint32_t));
		if (len <= 3) {
			if (len < 0) {
				if (_connection.getConnectionStatus() == enet::Tcp::status::link) {
					ENET_ERROR("Protocol error occured ...");
				}
				ENET_VERBOSE("ReadRaw 1 [STOP]");
				m_interface->stop(true);
				return;
			}
			ENET_ERROR("Time out ... ==> not managed ...");
			ENET_VERBOSE("ReadRaw 2 [STOP]");
			return;
		}
	}
	m_buffer.resize(totalSize);
	if (totalSize > 0) {
		uint64_t offset = 0;
		while (offset != totalSize) {
			len = _connection.read(&m_buffer[offset], totalSize-offset);
			offset += len;
			if (len == 0) {
				ENET_WARNING("Read No data");
			}
			if (len < 0) {
				m_interface->stop(true);
				return;
			}
		}
		// Need apply the mask:
		if ((size1 & 0x80) != 0) {
			for (size_t iii= 0; iii<m_buffer.size(); ++iii) {
				m_buffer[iii] ^= dataMask[iii%4];
			}
		}
	}
	
	// check opcode:
	if ((opcode & 0x0F) == enet::websocket::OPCODE_FRAME_CLOSE) {
		// Close the conection by remote:
		ENET_INFO("Close connection by remote :");
		m_interface->stop(true);
		return;
	}
	if ((opcode & 0x0F) == enet::websocket::OPCODE_FRAME_PING) {
		// Close the conection by remote:
		ENET_DEBUG("Receive a ping (send a pong)");
		controlPong();
		return;
	}
	if ((opcode & 0x0F) == enet::websocket::OPCODE_FRAME_PONG) {
		// Close the conection by remote:
		ENET_DEBUG("Receive a pong");
		return;
	}
	if ((opcode & 0x0F) == enet::websocket::OPCODE_FRAME_TEXT) {
		// Close the conection by remote:
		ENET_DEBUG("Receive a Text(UTF-8) data " << m_buffer.size() << " Bytes");
		if (m_observer != nullptr) {
			m_observer(m_buffer, true);
		}
		return;
	}
	if ((opcode & 0x0F) == enet::websocket::OPCODE_FRAME_BINARY) {
		// Close the conection by remote:
		if (m_observer != nullptr) {
			m_observer(m_buffer, false);
		}
		return;
	}
	ENET_ERROR("ReadRaw [STOP] (no opcode manage ... " << int32_t(opcode & 0x0F));
}

static std::string removeStartAndStopSpace(const std::string& _value) {
	std::string out;
	out.reserve(_value.size());
	bool findSpace = false;
	for (auto &it : _value) {
		if (it != ' ') {
			if (    findSpace == true
			     && out.size() != 0) {
				out += ' ';
			}
			out += it;
			findSpace = false;
		} else {
			findSpace = true;
		}
	}
	return out;
}

void enet::WebSocket::onReceiveRequest(const enet::HttpRequest& _data) {
	ememory::SharedPtr<enet::HttpServer> interface = ememory::dynamicPointerCast<enet::HttpServer>(m_interface);
	if (interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	_data.display();
	if (_data.getType() != enet::HTTPReqType::HTTP_GET) {
		enet::HttpAnswer answer(enet::HTTPAnswerCode::c400_badRequest, "support only GET");
		answer.setProtocol(enet::HTTPProtocol::http_1_1);
		answer.setKey("Connection", "close");
		interface->setHeader(answer);
		interface->stop(true);
		return;
	}
	if (_data.getKey("Connection") == "close") {
		enet::HttpAnswer answer(enet::HTTPAnswerCode::c200_ok);
		answer.setProtocol(enet::HTTPProtocol::http_1_1);
		answer.setKey("Connection", "close");
		interface->setHeader(answer);
		interface->stop(true);
		return;
	}
	if (_data.getKey("Upgrade") != "websocket") {
		enet::HttpAnswer answer(enet::HTTPAnswerCode::c400_badRequest, "websocket support only with Upgrade: websocket");
		answer.setProtocol(enet::HTTPProtocol::http_1_1);
		answer.setKey("Connection", "close");
		interface->setHeader(answer);
		interface->stop(true);
		return;
	}
	if (_data.getKey("Sec-WebSocket-Key") == "") {
		enet::HttpAnswer answer(enet::HTTPAnswerCode::c400_badRequest, "websocket missing 'Sec-WebSocket-Key'");
		answer.setProtocol(enet::HTTPProtocol::http_1_1);
		answer.setKey("Connection", "close");
		interface->setHeader(answer);
		interface->stop(true);
		return;
	}
	// parse all protocols:
	std::vector<std::string> listProtocol;
	if (_data.getKey("Sec-WebSocket-Protocol") != "") {
		listProtocol = etk::split(_data.getKey("Sec-WebSocket-Protocol"),',');
		for (size_t iii=0; iii<listProtocol.size(); ++iii) {
			listProtocol[iii] = removeStartAndStopSpace(listProtocol[iii]);
		}
	}
	
	if (m_observerUriCheck != nullptr) {
		if (m_observerUriCheck(_data.getUri(), listProtocol) == false) {
			enet::HttpAnswer answer(enet::HTTPAnswerCode::c404_notFound);
			answer.setProtocol(enet::HTTPProtocol::http_1_1);
			answer.setKey("Connection", "close");
			interface->setHeader(answer);
			interface->stop(true);
			return;
		}
	}
	enet::HttpAnswer answer(enet::HTTPAnswerCode::c101_switchingProtocols);
	answer.setProtocol(enet::HTTPProtocol::http_1_1);
	answer.setKey("Upgrade", "websocket");
	answer.setKey("Connection", "Upgrade");
	std::string answerKey = generateCheckKey(_data.getKey("Sec-WebSocket-Key"));
	answer.setKey("Sec-WebSocket-Accept", answerKey);
	if (m_protocol != "") {
		answer.setKey("Sec-WebSocket-Protocol", m_protocol);
	}
	interface->setHeader(answer);
}

void enet::WebSocket::onReceiveAnswer(const enet::HttpAnswer& _data) {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	_data.display();
	if (_data.getErrorCode() != enet::HTTPAnswerCode::c101_switchingProtocols) {
		ENET_ERROR("change protocol has not been accepted ... " << _data.getErrorCode() << " with message : " << _data.getHelp());
		m_interface->stop(true);
		return;
	}
	if (_data.getKey("Connection") != "Upgrade") {
		ENET_ERROR("Missing key : 'Connection : Upgrade' get '" << _data.getKey("Connection") << "'");
		m_interface->stop(true);
		return;
	}
	if (_data.getKey("Upgrade") != "websocket") {
		ENET_ERROR("Missing key : 'Upgrade : websocket' get '" << _data.getKey("Upgrade") << "'");
		m_interface->stop(true);
		return;
	}
	// NOTE : This is a temporary magic check ...
	if (_data.getKey("Sec-WebSocket-Accept") != m_checkKey) {
		ENET_ERROR("Wrong key : 'Sec-WebSocket-Accept : xxx' get '" << _data.getKey("Sec-WebSocket-Accept") << "'");
		m_interface->stop(true);
		return;
	}
	setProtocol(_data.getKey("Sec-WebSocket-Protocol"));
	// TODO : Create a methode to check the current protocol ...
	// now we can release the client call connection ...
	m_connectionValidate = true;
}
#define ZEUS_BASE_OFFSET_HEADER (15)
bool enet::WebSocket::configHeader(bool _isString, bool _mask) {
	m_isString = _isString;
	m_haveMask = _mask;
	m_sendBuffer.clear();
	m_sendBuffer.resize(ZEUS_BASE_OFFSET_HEADER, 0);
	if (_mask == true) {
		std::random_device rd;
		// Engine
		std::mt19937 e2(rd());
		// Distribtuions
		std::uniform_real_distribution<> dist(0, 0xFF);
		m_dataMask[0] = uint8_t(dist(e2));
		m_dataMask[1] = uint8_t(dist(e2));
		m_dataMask[2] = uint8_t(dist(e2));
		m_dataMask[3] = uint8_t(dist(e2));
	}
	return true;
}

int32_t enet::WebSocket::writeData(uint8_t* _data, int32_t _len) {
	size_t offset = m_sendBuffer.size();
	m_sendBuffer.resize(offset + _len);
	memcpy(&m_sendBuffer[offset], _data, _len);
	return _len;
}

int32_t enet::WebSocket::send() {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return -1;
	}
	if (m_haveMask == true) {
		for (size_t iii=ZEUS_BASE_OFFSET_HEADER; iii<m_sendBuffer.size(); ++iii) {
			m_sendBuffer[iii] ^= m_dataMask[iii%4];
		}
	}
	uint8_t mask = 0;
	if (m_haveMask == true) {
		mask = enet::websocket::FLAG_MASK;
	}
	int32_t offsetStart = ZEUS_BASE_OFFSET_HEADER-1;
	if (m_haveMask == true ) {
		m_sendBuffer[offsetStart--] = m_dataMask[3];
		m_sendBuffer[offsetStart--] = m_dataMask[2];
		m_sendBuffer[offsetStart--] = m_dataMask[1];
		m_sendBuffer[offsetStart--] = m_dataMask[0];
	}
	int32_t messageSize = m_sendBuffer.size()-ZEUS_BASE_OFFSET_HEADER;
	if (messageSize < 126) {
		m_sendBuffer[offsetStart--] = messageSize | mask;
	} else if (messageSize < 65338) {
		offsetStart -= sizeof(uint16_t);
		uint16_t* size = (uint16_t*)(&m_sendBuffer[offsetStart+1]);
		*size = messageSize;
		m_sendBuffer[offsetStart--] = 126 | mask;
	} else {
		offsetStart -= sizeof(uint64_t);
		uint64_t* size = (uint64_t*)(&m_sendBuffer[offsetStart+1]);
		*size = messageSize;
		m_sendBuffer[offsetStart--] = 127 | mask;
	}
	uint8_t header = enet::websocket::FLAG_FIN;
	if (m_isString == false) {
		header |= enet::websocket::OPCODE_FRAME_BINARY;
	} else {
		header |= enet::websocket::OPCODE_FRAME_TEXT;
	}
	m_sendBuffer[offsetStart] = header;
	//ENET_VERBOSE("buffersize=" << messageSize << " + " << ZEUS_BASE_OFFSET_HEADER-offsetStart);
	int32_t val = m_interface->write(&m_sendBuffer[offsetStart], m_sendBuffer.size()-offsetStart);
	m_sendBuffer.clear();
	m_sendBuffer.resize(ZEUS_BASE_OFFSET_HEADER, 0);
	return val;
}

int32_t enet::WebSocket::write(const void* _data, int32_t _len, bool _isString, bool _mask) {
	if (configHeader(_isString, _mask) == false) {
		return -1;
	}
	writeData((uint8_t*)_data, _len);
	return send();
}

void enet::WebSocket::controlPing() {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	uint8_t header =   enet::websocket::FLAG_FIN
	                 | enet::websocket::OPCODE_FRAME_PING;
	m_lastSend = std::chrono::steady_clock::now();
	m_interface->write(&header, sizeof(uint8_t));
	header = 0;
	m_interface->write(&header, sizeof(uint8_t));
}

void enet::WebSocket::controlPong() {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	uint8_t header =   enet::websocket::FLAG_FIN
	                 | enet::websocket::OPCODE_FRAME_PONG;
	m_lastSend = std::chrono::steady_clock::now();
	m_interface->write(&header, sizeof(uint8_t));
	header = 0;
	m_interface->write(&header, sizeof(uint8_t));
}

void enet::WebSocket::controlClose() {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	uint8_t header =   enet::websocket::FLAG_FIN
	                 | enet::websocket::OPCODE_FRAME_CLOSE;
	m_lastSend = std::chrono::steady_clock::now();
	m_interface->write(&header, sizeof(uint8_t));
	header = 0;
	m_interface->write(&header, sizeof(uint8_t));
}

