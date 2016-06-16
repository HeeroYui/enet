/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/debug.h>
#include <enet/Http.h>
#include <map>
#include <etk/stdTools.h>
#include <string.h>
#include <unistd.h>

static std::map<enet::HTTPAnswerCode, std::string> protocolName = {
	{enet::HTTPAnswerCode::c100_continue, "Continue"},
	{enet::HTTPAnswerCode::c101_switchingProtocols, "Switching Protocols"},
	{enet::HTTPAnswerCode::c103_checkpoint, "Checkpoint"},
	{enet::HTTPAnswerCode::c200_ok, "OK"},
	{enet::HTTPAnswerCode::c201_created, "Created"},
	{enet::HTTPAnswerCode::c202_accepted, "Accepted"},
	{enet::HTTPAnswerCode::c203_nonAuthoritativeInformation, "Non-Authoritative Information"},
	{enet::HTTPAnswerCode::c204_noContent, "No Content"},
	{enet::HTTPAnswerCode::c205_resetContent, "Reset Content"},
	{enet::HTTPAnswerCode::c206_partialContent, "Partial Content"},
	{enet::HTTPAnswerCode::c300_multipleChoices, "Multiple Choices"},
	{enet::HTTPAnswerCode::c301_movedPermanently, "Moved Permanently"},
	{enet::HTTPAnswerCode::c302_found, "Found"},
	{enet::HTTPAnswerCode::c303_seeOther, "See Other"},
	{enet::HTTPAnswerCode::c304_notModified, "Not Modified"},
	{enet::HTTPAnswerCode::c306_switchProxy, "Switch Proxy"},
	{enet::HTTPAnswerCode::c307_temporaryRedirect, "Temporary Redirect"},
	{enet::HTTPAnswerCode::c308_resumeIncomplete, "Resume Incomplete"},
	{enet::HTTPAnswerCode::c400_badRequest, "Bad Request"},
	{enet::HTTPAnswerCode::c401_unauthorized, "Unauthorized"},
	{enet::HTTPAnswerCode::c402_paymentRequired, "Payment Required"},
	{enet::HTTPAnswerCode::c403_forbidden, "Forbidden"},
	{enet::HTTPAnswerCode::c404_notFound, "Not Found"},
	{enet::HTTPAnswerCode::c405_methodNotAllowed, "Method Not Allowed"},
	{enet::HTTPAnswerCode::c406_notAcceptable, "Not Acceptable"},
	{enet::HTTPAnswerCode::c407_proxyAuthenticationRequired, "Proxy Authentication Required"},
	{enet::HTTPAnswerCode::c408_requestTimeout, "Request Timeout"},
	{enet::HTTPAnswerCode::c409_conflict, "Conflict"},
	{enet::HTTPAnswerCode::c410_gone, "Gone"},
	{enet::HTTPAnswerCode::c411_lengthRequired, "Length Required"},
	{enet::HTTPAnswerCode::c412_preconditionFailed, "Precondition Failed"},
	{enet::HTTPAnswerCode::c413_requestEntityTooLarge, "Request Entity Too Large"},
	{enet::HTTPAnswerCode::c414_requestURITooLong, "Request-URI Too Long"},
	{enet::HTTPAnswerCode::c415_unsupportedMediaType, "Unsupported Media Type"},
	{enet::HTTPAnswerCode::c416_requestedRangeNotSatisfiable, "Requested Range Not Satisfiable"},
	{enet::HTTPAnswerCode::c417_expectationFailed, "Expectation Failed"},
	{enet::HTTPAnswerCode::c500_internalServerError, "Internal Server Error"},
	{enet::HTTPAnswerCode::c501_notImplemented, "Not Implemented"},
	{enet::HTTPAnswerCode::c502_badGateway, "Bad Gateway"},
	{enet::HTTPAnswerCode::c503_serviceUnavailable, "Service Unavailable"},
	{enet::HTTPAnswerCode::c504_gatewayTimeout, "Gateway Timeout"},
	{enet::HTTPAnswerCode::c505_httpVersionNotSupported, "HTTP Version Not Supported"},
	{enet::HTTPAnswerCode::c511_networkAuthenticationRequired, "Network Authentication Required"}
};



static std::map<int32_t, std::string> getErrorList() {
	static std::map<int32_t, std::string> g_list;
	return g_list;
}

enet::Http::Http(enet::Tcp _connection, bool _isServer) :
  m_isServer(_isServer),
  m_connection(std::move(_connection)),
  m_headerIsSend(false),
  m_thread(nullptr),
  m_threadRunning(false),
  m_keepAlive(false) {
	setSendHeaderProperties("User-Agent", "e-net (ewol network interface)");
	if (m_keepAlive == true) {
		setSendHeaderProperties("Connection", "Keep-Alive");
	}
}

enet::Http::~Http() {
	stop();
}

void enet::Http::threadCallback() {
	ENET_DEBUG("Start of thread HTTP");
	ethread::setName("TcpString-input");
	// get datas:
	while (    m_threadRunning == true
	        && m_connection.getConnectionStatus() == enet::Tcp::status::link) {
		// READ section data:
		if (m_headerIsSend == false) {
			getHeader();
			m_headerIsSend = true;
		}
		m_temporaryBuffer.resize(67000);
		int32_t len = m_connection.read(&m_temporaryBuffer[0], m_temporaryBuffer.size());
		if (len > 0) {
			ENET_INFO("Call client with datas ...");
			if (m_observer != nullptr) {
				m_observer(*this, m_temporaryBuffer);
			}
		}
	}
	m_threadRunning = false;
	ENET_DEBUG("End of thread HTTP");
}


void enet::Http::start() {
	ENET_DEBUG("connect [START]");
	m_threadRunning = true;
	m_thread = new std::thread([&](void *){ this->threadCallback();}, nullptr);
	if (m_thread == nullptr) {
		m_threadRunning = false;
		ENET_ERROR("creating callback thread!");
		return;
	}
	while (    m_threadRunning == true
	        && m_connection.getConnectionStatus() != enet::Tcp::status::link) {
		usleep(50000);
	}
	//ethread::setPriority(*m_receiveThread, -6);
	ENET_DEBUG("connect [STOP]");
}

void enet::Http::stop(bool _inThreadStop){
	ENET_DEBUG("disconnect [START]");
	m_threadRunning = false;
	/*
	if (m_connection.getConnectionStatus() == enet::Tcp::status::link) {
		uint32_t size = 0xFFFFFFFF;
		m_connection.write(&size, 4);
	}
	*/
	if (m_connection.getConnectionStatus() != enet::Tcp::status::unlink) {
		m_connection.unlink();
	}
	if (_inThreadStop == false) {
		if (m_thread != nullptr) {
			m_thread->join();
			delete m_thread;
			m_thread = nullptr;
		}
	}
	ENET_DEBUG("disconnect [STOP]");
}

void enet::Http::setSendHeaderProperties(const std::string& _key, const std::string& _val) {
	auto it = m_header.m_map.find(_key);
	if (it == m_header.m_map.end()) {
		m_header.m_map.insert(make_pair(_key, _val));
	} else {
		it->second = _val;
	}
}

std::string enet::Http::getSendHeaderProperties(const std::string& _key) {
	ENET_TODO("get header key=" << _key);
	return "";
}

std::string enet::Http::getReceiveHeaderProperties(const std::string& _key) {
	ENET_TODO("get header key=" << _key);
	return "";
}
void enet::Http::writeAnswerHeader(enum enet::HTTPAnswerCode _value) {
	std::string out;
	out = "HTTP/1.1 ";
	out += etk::to_string(int32_t(_value));
	auto it = protocolName.find(_value);
	if (it == protocolName.end() ) {
		out += " ???";
	} else {
		out += " " + it->second;
	}
	out += "\r\n\r\n";
	ENET_WARNING("Write header :" << out);
	write(out, false);
}

void enet::Http::getHeader() {
	ENET_VERBOSE("Read HTTP Header [START]");
	bool headerEnded = false;
	std::string header;
	while (m_connection.getConnectionStatus() == enet::Tcp::status::link) {
		char type;
		int32_t len = m_connection.read(&type, 1);
		if (len == 0) {
			usleep(1);
			continue;
		}
		header += type;
		if (    header.size() > 4
		     && header[header.size()-1] == '\n'
		     && header[header.size()-2] == '\r'
		     && header[header.size()-3] == '\n'
		     && header[header.size()-4] == '\r') {
			// Normal end case ...
			break;
		} else if (    header.size() > 2
		           && header[header.size()-1] == '\n'
		           && header[header.size()-2] == '\n') {
			// linux end case
			break;
		} else if (    header.size() > 2
		           && header[header.size()-1] == '\r'
		           && header[header.size()-2] == '\r') {
			// Mac end case
			break;
		}
	}
	ENET_VERBOSE("Read HTTP Header [STOP] : '" << header << "'");
	m_headerIsSend = true;
	// parse header :
	std::vector<std::string> list = etk::split(header, '\n');
	for (auto &it : list) {
		if (    it.size()>0
		     && it[it.size()-1] == '\r') {
			it.resize(it.size()-1);
		}
	}
	headerEnded = false;
	m_header.m_map.clear();
	for (auto element : list) {
		if (headerEnded == false) {
			headerEnded = true;
			m_header.setReq(element);
		} else {
			size_t found = element.find(":");
			if (found == std::string::npos) {
				// nothing
				continue;
			}
			ENET_VERBOSE("header : key='" << std::string(element, 0, found) << "' value='" << std::string(element, found+2) << "'");
			m_header.m_map.insert(make_pair(unEscapeChar(std::string(element, 0, found)), unEscapeChar(std::string(element, found+2))));
		}
	}
	for (auto &it : m_header.m_map) {
		if (it.first == "Connection") {
			if (it.second == "close") {
				ENET_DEBUG("connection closed by remote :");
				m_connection.unlink();
			} else {
				ENET_TODO("manage connection type : '" << it.second);
			}
		}
	}
	m_header.display();
	if (m_observerRequest != nullptr) {
		m_observerRequest(*this, m_header);
	}
}


bool enet::Http::get(const std::string& _address) {
	m_receiveData.clear();
	m_header.m_map.clear();
	std::string req = "GET http://" + m_connection.getName();
	if (_address != "") {
		req += "/";
		req += _address;
	}
	req += " HTTP/1.1\r\n";
	setSendHeaderProperties("Content-Length", "0");
	// add header properties :
	for (auto &it : m_header.m_map) {
		req += escapeChar(it.first) + ": " + escapeChar(it.second) + "\r\n";
	}
	// end of header
	req += "\r\n";
	// no body:
	
	int32_t len = m_connection.write(req, false);
	ENET_VERBOSE("read write=" << len << " data: " << req);
	if (len != req.size()) {
		ENET_ERROR("An error occured when sending data " << len << "!=" << req.size());
		return false;
	}
	//return receiveData();
	return false;
}

std::string enet::Http::escapeChar(const std::string& _value) {
	return _value;
}
std::string enet::Http::unEscapeChar(const std::string& _value) {
	return _value;
}

bool enet::Http::post(const std::string& _address, const std::map<std::string, std::string>& _values) {
	m_receiveData.clear();
	m_header.m_map.clear();
	// First create body :
	std::string body;
	for (auto &it : _values) {
		if (body.size() > 0) {
			body += "&";
		}
		body += escapeChar(it.first) + "=" + escapeChar(it.second);
	}
	return post(_address, "application/x-www-form-urlencoded", body);
}

bool enet::Http::post(const std::string& _address, const std::string& _contentType, const std::string& _data) {
	m_receiveData.clear();
	m_header.m_map.clear();
	std::string req = "POST http://" + m_connection.getName();
	if (_address != "") {
		req += "/";
		req += _address;
	}
	req += " HTTP/1.0\n";
	setSendHeaderProperties("Content-Type", _contentType);
	setSendHeaderProperties("Content-Length", etk::to_string(_data.size()));
	// add header properties :
	for (auto &it : m_header.m_map) {
		req += escapeChar(it.first) + ": " + escapeChar(it.second) + "\r\n";
	}
	// end of header
	req += "\n";
	req += _data;
	
	int32_t len = m_connection.write(req, false);
	ENET_VERBOSE("read write=" << len << " data: " << req);
	if (len != req.size()) {
		ENET_ERROR("An error occured when sending data " << len << "!=" << req.size());
		return false;
	}
	//return receiveData();
	return false;
}


std::string enet::Http::dataString() {
	std::string data;
	for (auto element : m_receiveData) {
		if (element == '\0') {
			return data;
		}
		data += element;
	}
	return data;
}

int32_t enet::Http::write(const void* _data, int32_t _len) {
	return m_connection.write(_data, _len);
}

void enet::HttpHeader::setReq(const std::string& _req) {
	// parse base answear:
	std::vector<std::string> list = etk::split(_req, ' ');
	if (list.size() < 2) {
		ENET_ERROR("can not parse answear : " << list);
		return;
	}
	m_req = list[0];
	m_what = list[1];
	if (    m_req == "GET"
	     || m_req == "POST") {
		// HTTP CALL
		
	} else if (etk::start_with(m_req,"HTTP/")==true) {
		// HTTP answer
		int32_t ret = etk::string_to_int32_t(m_what);
		switch (ret/100) {
			case 1:
				// information message
				break;
			case 2:
				// OK
				break;
			case 3:
				// Redirect
				ENET_WARNING("Rediret request");
				break;
			case 4:
				// client Error
				ENET_WARNING("Client error");
				break;
			case 5:
				// server error
				ENET_WARNING("Server error");
				break;
		}
	}
}


void enet::HttpHeader::display() const {
	ENET_INFO("header :");
	for (auto &it : m_map) {
		ENET_INFO("    key='" << it.first << "' value='" << it.second << "'");
	}
}


