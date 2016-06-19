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
#include <random>


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

enet::WebSocket::WebSocket(enet::Tcp _connection, bool _isServer) :
  m_interface(),
  m_observer(nullptr),
  m_observerUriCheck(nullptr) {
	if (_isServer == true) {
		ememory::SharedPtr<enet::HttpServer> interface = std::make_shared<enet::HttpServer>(std::move(_connection));
		interface->connectHeader(this, &enet::WebSocket::onReceiveRequest);
		m_interface = interface;
	} else {
		ememory::SharedPtr<enet::HttpClient> interface = std::make_shared<enet::HttpClient>(std::move(_connection));
		interface->connectHeader(this, &enet::WebSocket::onReceiveAnswer);
		m_interface = interface;
	}
	m_interface->connectRaw(this, &enet::WebSocket::onReceiveData);
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
		req.setKey("Upgrade", "websocket");
		req.setKey("Connection", "Upgrade");
		req.setKey("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ=="); // this is an example key ...
		req.setKey("Sec-WebSocket-Version", "13");
		ememory::SharedPtr<enet::HttpClient> interface = std::dynamic_pointer_cast<enet::HttpClient>(m_interface);
		if (interface!=nullptr) {
			interface->setHeader(req);
		}
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

void enet::WebSocket::onReceiveData(enet::Tcp& _connection) {
	ENET_VERBOSE("Read Binary [START]");
	uint8_t opcode = 0;
	int32_t len = _connection.read(&opcode, sizeof(uint8_t));
	if (len <= 0) {
		if (len < 0) {
			ENET_ERROR("Protocol error occured ...");
			ENET_VERBOSE("ReadRaw 1 [STOP]");
			m_interface->stop(true);
			return;
		}
		ENET_ERROR("Time out ... ==> not managed ...");
		ENET_VERBOSE("ReadRaw 2 [STOP]");
		return;
	}
	ENET_VERBOSE("Read opcode : " << uint32_t(opcode));
	if ((opcode & 0x80) == 0) {
		ENET_ERROR("Multiple frames ... NOT managed ...");
		m_interface->stop(true);
		return;
	}
	int8_t size1 = 0;
	len = _connection.read(&size1, sizeof(uint8_t));
	ENET_VERBOSE("Read payload : " << uint32_t(size1));
	if (len <= 0) {
		if (len < 0) {
			ENET_ERROR("Protocol error occured ...");
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
				ENET_ERROR("Protocol error occured ...");
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
				ENET_ERROR("Protocol error occured ...");
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
				ENET_ERROR("Protocol error occured ...");
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
		ENET_INFO("Receive a ping (send a pong)");
		controlPong();
		return;
	}
	if ((opcode & 0x0F) == enet::websocket::OPCODE_FRAME_PONG) {
		// Close the conection by remote:
		ENET_INFO("Receive a pong");
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
		ENET_DEBUG("Receive a binary data " << m_buffer.size() << " Bytes");
		if (m_observer != nullptr) {
			m_observer(m_buffer, false);
		}
		return;
	}
	ENET_ERROR("ReadRaw [STOP] (no opcode manage ... " << int32_t(opcode & 0x0F));
	
}

void enet::WebSocket::onReceiveRequest(const enet::HttpRequest& _data) {
	ememory::SharedPtr<enet::HttpServer> interface = std::dynamic_pointer_cast<enet::HttpServer>(m_interface);
	if (interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	_data.display();
	if (_data.getType() != enet::HTTPReqType::GET) {
		enet::HttpAnswer answer(enet::HTTPAnswerCode::c400_badRequest, "support only GET");
		answer.setKey("Connection", "close");
		interface->setHeader(answer);
		interface->stop(true);
		return;
	}
	if (_data.getKey("Connection") == "close") {
		enet::HttpAnswer answer(enet::HTTPAnswerCode::c200_ok);
		answer.setKey("Connection", "close");
		interface->setHeader(answer);
		interface->stop(true);
		return;
	}
	if (_data.getKey("Upgrade") != "websocket") {
		enet::HttpAnswer answer(enet::HTTPAnswerCode::c400_badRequest, "websocket support only with Upgrade: websocket");
		answer.setKey("Connection", "close");
		interface->setHeader(answer);
		interface->stop(true);
		return;
	}
	if (_data.getKey("Sec-WebSocket-Key") == "") {
		enet::HttpAnswer answer(enet::HTTPAnswerCode::c400_badRequest, "websocket missing 'Sec-WebSocket-Key'");
		answer.setKey("Connection", "close");
		interface->setHeader(answer);
		interface->stop(true);
		return;
	}
	if (m_observerUriCheck != nullptr) {
		if (m_observerUriCheck(_data.getUri()) == false) {
			enet::HttpAnswer answer(enet::HTTPAnswerCode::c404_notFound);
			answer.setKey("Connection", "close");
			interface->setHeader(answer);
			interface->stop(true);
			return;
		}
	}
	enet::HttpAnswer answer(enet::HTTPAnswerCode::c101_switchingProtocols);
	answer.setKey("Upgrade", "websocket");
	answer.setKey("Connection", "Upgrade");
	// TODO: Do it better:
	answer.setKey("Sec-WebSocket-Accept", "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="); //base64::encode());
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
	if (_data.getKey("Sec-WebSocket-Accept") != "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") {
		ENET_ERROR("Wrong key : 'Sec-WebSocket-Accept : xxx' get '" << _data.getKey("Sec-WebSocket-Accept") << "'");
		m_interface->stop(true);
		return;
	}
	
}

int32_t enet::WebSocket::write(const void* _data, int32_t _len, bool _isString, bool _mask) {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return -1;
	}
	uint8_t mask = 0;
	if (_mask == true) {
		mask = enet::websocket::FLAG_MASK;
	}
	uint8_t header = enet::websocket::FLAG_FIN;
	if (_isString == false) {
		header |= enet::websocket::OPCODE_FRAME_BINARY;
	} else {
		header |= enet::websocket::OPCODE_FRAME_TEXT;
	}
	m_interface->write(&header, sizeof(uint8_t));
	ENET_VERBOSE("write opcode : " << int32_t(header));
	if (_len < 126) {
		uint8_t size = _len | mask;
		ENET_VERBOSE("write payload : " << int32_t(size));
		m_interface->write(&size, sizeof(uint8_t));
	} else if (_len < 65338) {
		uint8_t payload = 126 | mask;
		ENET_VERBOSE("write payload : " << int32_t(payload));
		m_interface->write(&payload, sizeof(uint8_t));
		uint16_t size = _len;
		ENET_VERBOSE("write size : " << int32_t(size));
		m_interface->write(&size, sizeof(uint16_t));
	} else {
		uint8_t payload = 127 | mask;
		ENET_VERBOSE("write payload : " << int32_t(payload));
		m_interface->write(&payload, sizeof(uint8_t));
		uint64_t size = _len;
		ENET_VERBOSE("write size : " << size);
		m_interface->write(&size, sizeof(uint64_t));
	}
	if (mask != 0 ) {
		std::random_device rd;
		// Engine
		std::mt19937 e2(rd());
		// Distribtuions
		std::uniform_real_distribution<> dist(0, 0xFF);
		uint8_t dataMask[4];
		dataMask[0] = uint8_t(dist(e2));
		dataMask[1] = uint8_t(dist(e2));
		dataMask[2] = uint8_t(dist(e2));
		dataMask[3] = uint8_t(dist(e2));
		m_interface->write(&dataMask, sizeof(uint32_t));
		std::vector<uint8_t> data;
		data.resize(_len);
		const uint8_t* pdata = static_cast<const uint8_t*>(_data);
		for (size_t iii= 0; iii<_len; ++iii) {
			data[iii] = pdata[iii] ^ dataMask[iii%4];
		}
		return m_interface->write(&data[0], data.size());
	}
	return m_interface->write(_data, _len);
}

void enet::WebSocket::controlPing() {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	uint16_t header = (  enet::websocket::FLAG_FIN
	                   | enet::websocket::OPCODE_FRAME_PING) << 8;
	m_interface->write(&header, sizeof(uint16_t));
}

void enet::WebSocket::controlPong() {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	uint16_t header = (  enet::websocket::FLAG_FIN
	                   | enet::websocket::OPCODE_FRAME_PONG) << 8;
	m_interface->write(&header, sizeof(uint16_t));
}

void enet::WebSocket::contolClose() {
	if (m_interface == nullptr) {
		ENET_ERROR("Nullptr interface ...");
		return;
	}
	uint16_t header = (  enet::websocket::FLAG_FIN
	                   | enet::websocket::OPCODE_FRAME_CLOSE) << 8;
	m_interface->write(&header, sizeof(uint16_t));
}

