/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/debug.hpp>
#include <enet/Http.hpp>
#include <map>
#include <etk/stdTools.hpp>
#include <cstring>


static std::string escapeChar(const std::string& _value) {
	return _value;
}
static std::string unEscapeChar(const std::string& _value) {
	return _value;
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
  m_threadRunning(false) {
	//setSendHeaderProperties("User-Agent", "e-net (ewol network interface)");
	/*
	if (m_keepAlive == true) {
		setSendHeaderProperties("Connection", "Keep-Alive");
	}
	*/
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
		if (m_observerRaw != nullptr) {
			m_observerRaw(m_connection);
		} else {
			m_temporaryBuffer.resize(67000);
			int32_t len = m_connection.read(&m_temporaryBuffer[0], m_temporaryBuffer.size());
			if (len > 0) {
				ENET_INFO("Call client with datas ...");
				if (m_observer != nullptr) {
					m_observer(m_temporaryBuffer);
				}
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
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
/*
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
*/
namespace etk {
	template <>
	bool from_string<enum enet::HTTPAnswerCode>(enum enet::HTTPAnswerCode& _variableRet, const std::string& _value) {
		_variableRet = enet::HTTPAnswerCode::c000_unknow;
		for (auto &it : protocolName) {
			if (etk::to_string(int32_t(it.first)) == _value) {
				_variableRet = it.first;
				return true;
			}
		}
		return false;
	}
	template <>
	std::string to_string<enum enet::HTTPAnswerCode>(const enum enet::HTTPAnswerCode& _value) {
		return etk::to_string(int32_t(_value));
	}
	template <>
	bool from_string<enum enet::HTTPReqType>(enum enet::HTTPReqType& _variableRet, const std::string& _value) {
		_variableRet = enet::HTTPReqType::HTTP_GET;
		if (_value == "GET") {
			_variableRet = enet::HTTPReqType::HTTP_GET;
			return true;
		} else if (_value == "HEAD") {
			_variableRet = enet::HTTPReqType::HTTP_HEAD;
			return true;
		} else if (_value == "POST") {
			_variableRet = enet::HTTPReqType::HTTP_POST;
			return true;
		} else if (_value == "PUT") {
			_variableRet = enet::HTTPReqType::HTTP_PUT;
			return true;
		} else if (_value == "DELETE") {
			_variableRet = enet::HTTPReqType::HTTP_DELETE;
			return true;
		}
		return false;
	}
	template <>
	std::string to_string<enum enet::HTTPReqType>(const enum enet::HTTPReqType& _value) {
		switch (_value) {
			case enet::HTTPReqType::HTTP_GET: return "GET";
			case enet::HTTPReqType::HTTP_HEAD: return "HEAD";
			case enet::HTTPReqType::HTTP_POST: return "POST";
			case enet::HTTPReqType::HTTP_PUT: return "PUT";
			case enet::HTTPReqType::HTTP_DELETE: return "DELETE";
		}
		return "UNKNOW";
	}
	template <>
	bool from_string<enum enet::HTTPProtocol>(enum enet::HTTPProtocol& _variableRet, const std::string& _value) {
		_variableRet = enet::HTTPProtocol::http_0_1;
		if (_value == "HTTP/0.1") { _variableRet = enet::HTTPProtocol::http_0_1; return true; }
		if (_value == "HTTP/0.2") { _variableRet = enet::HTTPProtocol::http_0_2; return true; }
		if (_value == "HTTP/0.3") { _variableRet = enet::HTTPProtocol::http_0_3; return true; }
		if (_value == "HTTP/0.4") { _variableRet = enet::HTTPProtocol::http_0_4; return true; }
		if (_value == "HTTP/0.5") { _variableRet = enet::HTTPProtocol::http_0_5; return true; }
		if (_value == "HTTP/0.6") { _variableRet = enet::HTTPProtocol::http_0_6; return true; }
		if (_value == "HTTP/0.7") { _variableRet = enet::HTTPProtocol::http_0_7; return true; }
		if (_value == "HTTP/0.8") { _variableRet = enet::HTTPProtocol::http_0_8; return true; }
		if (_value == "HTTP/0.9") { _variableRet = enet::HTTPProtocol::http_0_9; return true; }
		if (_value == "HTTP/0.10") { _variableRet = enet::HTTPProtocol::http_0_10; return true; }
		if (_value == "HTTP/1.0") { _variableRet = enet::HTTPProtocol::http_1_0; return true; }
		if (_value == "HTTP/1.1") { _variableRet = enet::HTTPProtocol::http_1_1; return true; }
		if (_value == "HTTP/1.2") { _variableRet = enet::HTTPProtocol::http_1_2; return true; }
		if (_value == "HTTP/1.3") { _variableRet = enet::HTTPProtocol::http_1_3; return true; }
		if (_value == "HTTP/1.4") { _variableRet = enet::HTTPProtocol::http_1_4; return true; }
		if (_value == "HTTP/1.5") { _variableRet = enet::HTTPProtocol::http_1_5; return true; }
		if (_value == "HTTP/1.6") { _variableRet = enet::HTTPProtocol::http_1_6; return true; }
		if (_value == "HTTP/1.7") { _variableRet = enet::HTTPProtocol::http_1_7; return true; }
		if (_value == "HTTP/1.8") { _variableRet = enet::HTTPProtocol::http_1_8; return true; }
		if (_value == "HTTP/1.9") { _variableRet = enet::HTTPProtocol::http_1_9; return true; }
		if (_value == "HTTP/1.10") { _variableRet = enet::HTTPProtocol::http_1_10; return true; }
		if (_value == "HTTP/2.0") { _variableRet = enet::HTTPProtocol::http_2_0; return true; }
		if (_value == "HTTP/2.1") { _variableRet = enet::HTTPProtocol::http_2_1; return true; }
		if (_value == "HTTP/2.2") { _variableRet = enet::HTTPProtocol::http_2_2; return true; }
		if (_value == "HTTP/2.3") { _variableRet = enet::HTTPProtocol::http_2_3; return true; }
		if (_value == "HTTP/2.4") { _variableRet = enet::HTTPProtocol::http_2_4; return true; }
		if (_value == "HTTP/2.5") { _variableRet = enet::HTTPProtocol::http_2_5; return true; }
		if (_value == "HTTP/2.6") { _variableRet = enet::HTTPProtocol::http_2_6; return true; }
		if (_value == "HTTP/2.7") { _variableRet = enet::HTTPProtocol::http_2_7; return true; }
		if (_value == "HTTP/2.8") { _variableRet = enet::HTTPProtocol::http_2_8; return true; }
		if (_value == "HTTP/2.9") { _variableRet = enet::HTTPProtocol::http_2_9; return true; }
		if (_value == "HTTP/2.10") { _variableRet = enet::HTTPProtocol::http_2_10; return true; }
		if (_value == "HTTP/3.0") { _variableRet = enet::HTTPProtocol::http_3_0; return true; }
		if (_value == "HTTP/3.1") { _variableRet = enet::HTTPProtocol::http_3_1; return true; }
		if (_value == "HTTP/3.2") { _variableRet = enet::HTTPProtocol::http_3_2; return true; }
		if (_value == "HTTP/3.3") { _variableRet = enet::HTTPProtocol::http_3_3; return true; }
		if (_value == "HTTP/3.4") { _variableRet = enet::HTTPProtocol::http_3_4; return true; }
		if (_value == "HTTP/3.5") { _variableRet = enet::HTTPProtocol::http_3_5; return true; }
		if (_value == "HTTP/3.6") { _variableRet = enet::HTTPProtocol::http_3_6; return true; }
		if (_value == "HTTP/3.7") { _variableRet = enet::HTTPProtocol::http_3_7; return true; }
		if (_value == "HTTP/3.8") { _variableRet = enet::HTTPProtocol::http_3_8; return true; }
		if (_value == "HTTP/3.9") { _variableRet = enet::HTTPProtocol::http_3_9; return true; }
		if (_value == "HTTP/3.10") { _variableRet = enet::HTTPProtocol::http_3_10; return true; }
		return false;
	}
	template <>
	std::string to_string<enum enet::HTTPProtocol>(const enum enet::HTTPProtocol& _value) {
		switch (_value) {
			case enet::HTTPProtocol::http_0_1:  return "HTTP/0.1";
			case enet::HTTPProtocol::http_0_2:  return "HTTP/0.2";
			case enet::HTTPProtocol::http_0_3:  return "HTTP/0.3";
			case enet::HTTPProtocol::http_0_4:  return "HTTP/0.4";
			case enet::HTTPProtocol::http_0_5:  return "HTTP/0.5";
			case enet::HTTPProtocol::http_0_6:  return "HTTP/0.6";
			case enet::HTTPProtocol::http_0_7:  return "HTTP/0.7";
			case enet::HTTPProtocol::http_0_8:  return "HTTP/0.8";
			case enet::HTTPProtocol::http_0_9:  return "HTTP/0.9";
			case enet::HTTPProtocol::http_0_10: return "HTTP/0.10";
			case enet::HTTPProtocol::http_1_0:  return "HTTP/1.0";
			case enet::HTTPProtocol::http_1_1:  return "HTTP/1.1";
			case enet::HTTPProtocol::http_1_2:  return "HTTP/1.2";
			case enet::HTTPProtocol::http_1_3:  return "HTTP/1.3";
			case enet::HTTPProtocol::http_1_4:  return "HTTP/1.4";
			case enet::HTTPProtocol::http_1_5:  return "HTTP/1.5";
			case enet::HTTPProtocol::http_1_6:  return "HTTP/1.6";
			case enet::HTTPProtocol::http_1_7:  return "HTTP/1.7";
			case enet::HTTPProtocol::http_1_8:  return "HTTP/1.8";
			case enet::HTTPProtocol::http_1_9:  return "HTTP/1.9";
			case enet::HTTPProtocol::http_1_10: return "HTTP/1.10";
			case enet::HTTPProtocol::http_2_0:  return "HTTP/2.0";
			case enet::HTTPProtocol::http_2_1:  return "HTTP/2.1";
			case enet::HTTPProtocol::http_2_2:  return "HTTP/2.2";
			case enet::HTTPProtocol::http_2_3:  return "HTTP/2.3";
			case enet::HTTPProtocol::http_2_4:  return "HTTP/2.4";
			case enet::HTTPProtocol::http_2_5:  return "HTTP/2.5";
			case enet::HTTPProtocol::http_2_6:  return "HTTP/2.6";
			case enet::HTTPProtocol::http_2_7:  return "HTTP/2.7";
			case enet::HTTPProtocol::http_2_8:  return "HTTP/2.8";
			case enet::HTTPProtocol::http_2_9:  return "HTTP/2.9";
			case enet::HTTPProtocol::http_2_10: return "HTTP/2.10";
			case enet::HTTPProtocol::http_3_0:  return "HTTP/3.0";
			case enet::HTTPProtocol::http_3_1:  return "HTTP/3.1";
			case enet::HTTPProtocol::http_3_2:  return "HTTP/3.2";
			case enet::HTTPProtocol::http_3_3:  return "HTTP/3.3";
			case enet::HTTPProtocol::http_3_4:  return "HTTP/3.4";
			case enet::HTTPProtocol::http_3_5:  return "HTTP/3.5";
			case enet::HTTPProtocol::http_3_6:  return "HTTP/3.6";
			case enet::HTTPProtocol::http_3_7:  return "HTTP/3.7";
			case enet::HTTPProtocol::http_3_8:  return "HTTP/3.8";
			case enet::HTTPProtocol::http_3_9:  return "HTTP/3.9";
			case enet::HTTPProtocol::http_3_10: return "HTTP/3.10";
		}
		return "HTTP/0.1";
	}
}


void enet::Http::setRequestHeader(const enet::HttpRequest& _req) {
	m_requestHeader = _req;
	if (m_requestHeader.getKey("User-Agent") == "") {
		m_requestHeader.setKey("User-Agent", "e-net (ewol network interface)");
	}
	std::string value = m_requestHeader.generate();
	write(value, false);
}

void enet::Http::setAnswerHeader(const enet::HttpAnswer& _req) {
	m_answerHeader = _req;
	if (m_requestHeader.getKey("User-Agent") == "") {
		m_requestHeader.setKey("User-Agent", "e-net (ewol network interface)");
	}
	std::string value = m_answerHeader.generate();
	write(value, false);
}

void enet::Http::getHeader() {
	ENET_VERBOSE("Read HTTP Header [START]");
	bool headerEnded = false;
	std::string header;
	while (m_connection.getConnectionStatus() == enet::Tcp::status::link) {
		char type;
		int32_t len = m_connection.read(&type, 1);
		if (len == 0) {
			std::this_thread::sleep_for(std::chrono::microseconds(1));
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
	if (m_connection.getConnectionStatus() != enet::Tcp::status::link) {
		ENET_ERROR("Read HTTP Header [STOP] : '" << header << "' ==> status move in unlink ...");
		return;
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
	//parse first element:
	std::vector<std::string> listLineOne = etk::split(list[0], ' ');
	if (listLineOne.size() < 2) {
		ENET_ERROR("can not parse answear : " << listLineOne);
		// answer bad request and close connection ...
		
		return;
	}
	if (    listLineOne.size() >= 3
	     && (    listLineOne[0] == "GET"
	          || listLineOne[0] == "POST"
	          || listLineOne[0] == "HEAD"
	          || listLineOne[0] == "DELETE"
	          || listLineOne[0] == "PUT" ) ) {
		// HTTP CALL
		if (m_isServer == false) {
			// can not have call in client mode
			ENET_ERROR("can not parse call in client mode ..." << listLineOne);
			m_answerHeader.setErrorCode(enet::HTTPAnswerCode::c400_badRequest);
			m_answerHeader.setHelp("Call a client with a request from server ...");
			setAnswerHeader(m_answerHeader);
			stop(true);
			return;
		}
		// get type call:
		enum enet::HTTPReqType valueType;
		etk::from_string(valueType, listLineOne[0]);
		m_requestHeader.setType(valueType);
		// get URI:
		m_requestHeader.setUri(listLineOne[1]);
		// Get http version:
		enum enet::HTTPProtocol valueProtocol;
		etk::from_string(valueProtocol, listLineOne[2]);
		m_requestHeader.setProtocol(valueProtocol);
	} else if (    listLineOne.size() >= 3
	            && etk::start_with(listLineOne[0],"HTTP/") == true) {
		// HTTP answer
		if (m_isServer == true) {
			// can not have anser ==> need to be a get ot something like this ...
			ENET_ERROR("can not parse answer in server mode ..." << listLineOne);
			m_answerHeader.setErrorCode(enet::HTTPAnswerCode::c400_badRequest);
			m_answerHeader.setHelp("Call a client with a request from server ...");
			setAnswerHeader(m_answerHeader);
			stop(true);
			return;
		}
		// Get http version:
		enum enet::HTTPProtocol valueProtocol;
		etk::from_string(valueProtocol, listLineOne[0]);
		m_answerHeader.setProtocol(valueProtocol);
		
		enum HTTPAnswerCode valueErrorCode;
		etk::from_string(valueErrorCode, listLineOne[1]);
		m_answerHeader.setErrorCode(valueErrorCode);
		
		// get comment:
		std::string comment;
		for (size_t iii=2; iii<listLineOne.size(); ++iii) {
			if (comment.size() != 0) {
				comment += " ";
			}
			comment += listLineOne[iii];
		}
		m_answerHeader.setHelp(comment);
	} else {
		// can not have anser ==> need to be a get ot something like this ...
		ENET_ERROR("Un understand message ..." << listLineOne);
		m_answerHeader.setErrorCode(enet::HTTPAnswerCode::c400_badRequest);
		m_answerHeader.setHelp("Un understand message ...");
		setAnswerHeader(m_answerHeader);
		stop(true);
		return;
	}
	for (size_t iii=1; iii<list.size(); ++iii) {
		size_t found = list[iii].find(":");
		if (found == std::string::npos) {
			// nothing
			continue;
		}
		std::string key = unEscapeChar(std::string(list[iii], 0, found));
		key = removeStartAndStopSpace(key);
		std::string value = unEscapeChar(std::string(list[iii], found+2));
		value = removeStartAndStopSpace(value);
		ENET_VERBOSE("header : key='" << key << "' value='" << value << "'");
		if (m_isServer == false) {
			m_answerHeader.setKey(key,value);
		} else {
			m_requestHeader.setKey(key,value);
		}
		if (    key == "Connection"
		     && value == "close") {
			ENET_DEBUG("connection closed by remote :");
			m_connection.unlink();
		}
	}
	if (m_isServer == false) {
		if (m_observerAnswer != nullptr) {
			m_observerAnswer(m_answerHeader);
		}
	} else {
		if (m_observerRequest != nullptr) {
			m_observerRequest(m_requestHeader);
		}
	}
}


/*
bool enet::Http::get(const std::string& _address) {
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

bool enet::Http::post(const std::string& _address, const std::map<std::string, std::string>& _values) {
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
*/

int32_t enet::Http::write(const void* _data, int32_t _len) {
	return m_connection.write(_data, _len);
}


void enet::HttpHeader::setKey(const std::string& _key, const std::string& _value) {
	auto it = m_map.find(_key);
	if (it == m_map.end()) {
		m_map.insert(make_pair(_key, _value));
	} else {
		it->second = _value;
	}
}

void enet::HttpHeader::rmKey(const std::string& _key) {
	auto it = m_map.find(_key);
	if (it != m_map.end()) {
		m_map.erase(it);
	}
}

std::string enet::HttpHeader::getKey(const std::string& _key) const {
	auto it = m_map.find(_key);
	if (it != m_map.end()) {
		return it->second;
	}
	return "";
}

std::string enet::HttpHeader::generateKeys() const {
	std::string out;
	for (auto &it : m_map) {
		if (    it.first != ""
		     && it.second != "") {
			out += escapeChar(it.first) + " : " + escapeChar(it.second) + "\r\n";
		}
	}
	return out;
}

enet::HttpHeader::HttpHeader():
  m_protocol(enet::HTTPProtocol::http_1_0) {
	
}


// -----------------------------------------------------------------------------------------


enet::HttpAnswer::HttpAnswer(enum HTTPAnswerCode _code, const std::string& _help):
  m_what(_code),
  m_helpMessage(_help) {
	
}

void enet::HttpAnswer::display() const {
	ENET_PRINT("display header 'Answer' ");
	ENET_PRINT("    protocol=" << m_protocol);
	ENET_PRINT("    Code=" << int32_t(m_what));
	ENET_PRINT("    message=" << m_helpMessage);
	ENET_PRINT("    Options:");
	for (auto &it : m_map) {
		if (    it.first != ""
		     && it.second != "") {
			ENET_PRINT("        '" + it.first + "' = '" + it.second + "'");
		}
	}
}

std::string enet::HttpAnswer::generate() const {
	std::string out;
	out += etk::to_string(m_protocol);
	out += " ";
	out += etk::to_string(int32_t(m_what));
	out += " ";
	if (m_helpMessage != "") {
		out += escapeChar(m_helpMessage);
	} else {
		auto it = protocolName.find(m_what);
		if (it != protocolName.end()) {
			out += escapeChar(it->second);
		} else {
			out += "???";
		}
	}
	out += "\r\n";
	out += generateKeys();
	out += "\r\n";
	return out;
}
enet::HttpServer::HttpServer(enet::Tcp _connection) :
  enet::Http(std::move(_connection), true) {
	
}

enet::HttpClient::HttpClient(enet::Tcp _connection) :
  enet::Http(std::move(_connection), false) {
	
}
// -----------------------------------------------------------------------------------------

enet::HttpRequest::HttpRequest(enum enet::HTTPReqType _type):
  m_req(_type),
  m_uri() {
	
}

void enet::HttpRequest::display() const {
	ENET_PRINT("display header 'Request' ");
	ENET_PRINT("    type=" << m_req);
	ENET_PRINT("    protocol=" << m_protocol);
	ENET_PRINT("    uri=" << m_uri);
	ENET_PRINT("    Options:");
	for (auto &it : m_map) {
		if (    it.first != ""
		     && it.second != "") {
			ENET_PRINT("        '" + it.first + "' = '" + it.second + "'");
		}
	}
}

std::string enet::HttpRequest::generate() const {
	std::string out;
	out += etk::to_string(m_req);
	out += " ";
	out += m_uri;
	out += " ";
	out += etk::to_string(m_protocol);
	out += "\r\n";
	out += generateKeys();
	out += "\r\n";
	return out;
}


std::ostream& enet::operator <<(std::ostream& _os, enum enet::HTTPProtocol _obj) {
	_os << "enet::HTTPProtocol::" <<etk::to_string(_obj);
	return _os;
}

std::ostream& enet::operator <<(std::ostream& _os, enum enet::HTTPAnswerCode _obj) {
	_os << "enet::HTTPAnswerCode::" << etk::to_string(_obj);
	return _os;
}

std::ostream& enet::operator <<(std::ostream& _os, enum enet::HTTPReqType _obj) {
	_os << "enet::HTTPReqType::" << etk::to_string(_obj);
	return _os;
}