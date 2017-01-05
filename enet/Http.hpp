/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <enet/Tcp.hpp>
#include <vector>
#include <map>
#include <thread>
#include <ethread/tools.hpp>

namespace enet {
	enum class HTTPAnswerCode {
		c000_unknow = 0,
		//1xx: Information
		c100_continue = 100, //!< The server has received the request headers, and the client should proceed to send the request body
		c101_switchingProtocols, //!< The requester has asked the server to switch protocols
		c103_checkpoint, //!< Used in the resumable requests proposal to resume aborted PUT or POST requests
		//2xx: Successful
		c200_ok = 200, //!< The request is OK (this is the standard response for successful HTTP requests)
		c201_created, //!< The request has been fulfilled, and a new resource is created 
		c202_accepted, //!< The request has been accepted for processing, but the processing has not been completed
		c203_nonAuthoritativeInformation, //!< The request has been successfully processed, but is returning information that may be from another source
		c204_noContent, //!< The request has been successfully processed, but is not returning any content
		c205_resetContent, //!< The request has been successfully processed, but is not returning any content, and requires that the requester reset the document view
		c206_partialContent, //!< The server is delivering only part of the resource due to a range header sent by the client
		//3xx: Redirection
		c300_multipleChoices = 300, //!< A link list. The user can select a link and go to that location. Maximum five addresses
		c301_movedPermanently, //!< The requested page has moved to a new URL 
		c302_found, //!< The requested page has moved temporarily to a new URL 
		c303_seeOther, //!< The requested page can be found under a different URL
		c304_notModified, //!< Indicates the requested page has not been modified since last requested
		c306_switchProxy, //!< No longer used
		c307_temporaryRedirect, //!< The requested page has moved temporarily to a new URL
		c308_resumeIncomplete, //!< Used in the resumable requests proposal to resume aborted PUT or POST requests
		//4xx: Client Error,
		c400_badRequest = 400, //!< The request cannot be fulfilled due to bad syntax
		c401_unauthorized, //!< The request was a legal request, but the server is refusing to respond to it. For use when authentication is possible but has failed or not yet been provided
		c402_paymentRequired, //!< Reserved for future use
		c403_forbidden, //!< The request was a legal request, but the server is refusing to respond to it
		c404_notFound, //!< The requested page could not be found but may be available again in the future
		c405_methodNotAllowed, //!< A request was made of a page using a request method not supported by that page
		c406_notAcceptable, //!< The server can only generate a response that is not accepted by the client
		c407_proxyAuthenticationRequired, //!< The client must first authenticate itself with the proxy
		c408_requestTimeout, //!< The server timed out waiting for the request
		c409_conflict, //!< The request could not be completed because of a conflict in the request
		c410_gone, //!< The requested page is no longer available
		c411_lengthRequired, //!< The "Content-Length" is not defined. The server will not accept the request without it
		c412_preconditionFailed, //!< The precondition given in the request evaluated to false by the server
		c413_requestEntityTooLarge, //!< The server will not accept the request, because the request entity is too large
		c414_requestURITooLong, //!< The server will not accept the request, because the URL is too long. Occurs when you convert a POST request to a GET request with a long query information
		c415_unsupportedMediaType, //!< The server will not accept the request, because the media type is not supported
		c416_requestedRangeNotSatisfiable, //!< The client has asked for a portion of the file, but the server cannot supply that portion
		c417_expectationFailed, //!< The server cannot meet the requirements of the Expect request-header field
		//5xx: Server Error
		c500_internalServerError = 500, //!< A generic error message, given when no more specific message is suitable
		c501_notImplemented, //!< The server either does not recognize the request method, or it lacks the ability to fulfill the request
		c502_badGateway, //!< The server was acting as a gateway or proxy and received an invalid response from the upstream server
		c503_serviceUnavailable, //!< The server is currently unavailable (overloaded or down)
		c504_gatewayTimeout, //!< The server was acting as a gateway or proxy and did not receive a timely response from the upstream server
		c505_httpVersionNotSupported, //!< The server does not support the HTTP protocol version used in the request
		c511_networkAuthenticationRequired, //!< The client needs to authenticate to gain network access
	};
	std::ostream& operator <<(std::ostream& _os, enum enet::HTTPAnswerCode _obj);
	
	enum class HTTPProtocol {
		http_0_1,
		http_0_2,
		http_0_3,
		http_0_4,
		http_0_5,
		http_0_6,
		http_0_7,
		http_0_8,
		http_0_9,
		http_0_10,
		http_1_0,
		http_1_1,
		http_1_2,
		http_1_3,
		http_1_4,
		http_1_5,
		http_1_6,
		http_1_7,
		http_1_8,
		http_1_9,
		http_1_10,
		http_2_0,
		http_2_1,
		http_2_2,
		http_2_3,
		http_2_4,
		http_2_5,
		http_2_6,
		http_2_7,
		http_2_8,
		http_2_9,
		http_2_10,
		http_3_0,
		http_3_1,
		http_3_2,
		http_3_3,
		http_3_4,
		http_3_5,
		http_3_6,
		http_3_7,
		http_3_8,
		http_3_9,
		http_3_10,
	};
	std::ostream& operator <<(std::ostream& _os, enum enet::HTTPProtocol _obj);
	class HttpHeader {
		protected:
			// key, val
			std::map<std::string, std::string> m_map;
			enum HTTPProtocol m_protocol;
		public:
			void setKey(const std::string& _key, const std::string& _value);
			void rmKey(const std::string& _key);
			std::string getKey(const std::string& _key) const;
		protected:
			std::string generateKeys() const;
		public:
			enum HTTPProtocol getProtocol() const {
				return m_protocol;
			}
			void setProtocol(enum HTTPProtocol _protocol) {
				m_protocol = _protocol;
			}
			HttpHeader();
			virtual ~HttpHeader() = default;
			virtual std::string generate() const = 0;
	};
	
	class HttpAnswer : public HttpHeader {
		private:
			enet::HTTPAnswerCode m_what;
			std::string m_helpMessage;
		public:
			HttpAnswer(enum HTTPAnswerCode _code = enet::HTTPAnswerCode::c400_badRequest, const std::string& _help="");
			void display() const;
			std::string generate() const;
			void setErrorCode(enum HTTPAnswerCode _value) {
				m_what = _value;
			}
			enum HTTPAnswerCode getErrorCode() const {
				return m_what;
			}
			void setHelp(const std::string& _value) {
				m_helpMessage = _value;
			}
			const std::string& getHelp() const {
				return m_helpMessage;
			}
	};
	enum class HTTPReqType {
		HTTP_GET,
		HTTP_HEAD,
		HTTP_POST,
		HTTP_PUT,
		HTTP_DELETE,
	};
	std::ostream& operator <<(std::ostream& _os, enum enet::HTTPReqType _obj);
	class HttpRequest : public HttpHeader {
		private:
			// key, val
			enum HTTPReqType m_req;
			std::string m_uri;
		public:
			HttpRequest(enum enet::HTTPReqType _type=enet::HTTPReqType::HTTP_GET);
			void display() const;
			std::string generate() const;
			void setType(enum enet::HTTPReqType _value) {
				m_req = _value;
			}
			enum enet::HTTPReqType getType() const{
				return m_req;
			}
			void setUri(const std::string& _value) {
				m_uri = _value;
			}
			const std::string& getUri() const {
				return m_uri;
			}
	};
	class Http {
		public:
			Http(enet::Tcp _connection, bool _isServer=false);
			virtual ~Http();
		private:
			bool m_isServer;
		public:
			bool getServerState() {
				return m_isServer;
			}
			bool isServer() {
				return m_isServer;
			}
		protected:
			enet::HttpRequest m_requestHeader;
			void setRequestHeader(const enet::HttpRequest& _req);
		public:
			const enet::HttpRequest& getRequestHeader() {
				return m_requestHeader;
			}
		protected:
			enet::HttpAnswer m_answerHeader;
			void setAnswerHeader(const enet::HttpAnswer& _req);
		public:
			const enet::HttpAnswer& getAnswerHeader() {
				return m_answerHeader;
			}
		protected:
			enet::Tcp m_connection;
			bool m_headerIsSend;
			std::thread* m_thread;
			bool m_threadRunning;
			std::vector<uint8_t> m_temporaryBuffer;
		private:
			void threadCallback();
		private:
			void getHeader();
		public:
			void start();
			void stop(bool _inThread=false);
			bool isAlive() const {
				return m_connection.getConnectionStatus() == enet::Tcp::status::link;
			}
		public:
			using Observer = std::function<void(std::vector<uint8_t>&)>; //!< Define an Observer: function pointer
			Observer m_observer;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(std::vector<uint8_t>&)) {
				m_observer = [=](std::vector<uint8_t>& _value){
					(*_class.*_func)(_value);
				};
			}
			void connect(Observer _func) {
				m_observer = _func;
			}
		public:
			using ObserverRaw = std::function<void(enet::Tcp&)>; //!< Define an Observer: function pointer
			ObserverRaw m_observerRaw;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connectRaw(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(enet::Tcp&)) {
				m_observerRaw = [=](enet::Tcp& _connection){
					(*_class.*_func)(_connection);
				};
			}
			void connectRaw(ObserverRaw _func) {
				m_observerRaw = _func;
			}
		public:
			using ObserverRequest = std::function<void(const enet::HttpRequest&)>; //!< Define an Observer: function pointer
		protected:
			ObserverRequest m_observerRequest;
		public:
			using ObserverAnswer = std::function<void(const enet::HttpAnswer&)>; //!< Define an Observer: function pointer
		protected:
			ObserverAnswer m_observerAnswer;
		public:
			/**
			 * @brief Write a chunk of data on the socket
			 * @param[in] _data pointer on the data might be write
			 * @param[in] _len Size that must be written socket
			 * @return >0 byte size on the socket write
			 * @return -1 an error occured.
			 */
			int32_t write(const void* _data, int32_t _len);
			/**
			 * @brief Write a chunk of data on the socket
			 * @param[in] _data String to rite on the soccket
			 * @param[in] _writeBackSlashZero if false, the \0 is not write
			 * @return >0 byte size on the socket write
			 * @return -1 an error occured.
			 */
			int32_t write(const std::string& _data, bool _writeBackSlashZero = true) {
				if (_data.size() == 0) {
					return 0;
				}
				if (_writeBackSlashZero == true) {
					return write(_data.c_str(), _data.size()+1);
				}
				return write(_data.c_str(), _data.size());
			}
			/**
			 * @brief Write a chunk of data on the socket
			 * @param[in] _data String to rite on the soccket
			 * @param[in] _writeBackSlashZero if false, the \0 is not write
			 * @return >0 T element write on the socket
			 * @return -1 an error occured.
			 */
			template <class T>
			int32_t write(const std::vector<T>& _data) {
				if (_data.size() == 0) {
					return 0;
				}
				size_t ret = write(&_data[0], _data.size()*sizeof(T));
				if (ret <=0) {
					return ret;
				}
				return ret/sizeof(T);
			}
	};
	
	class HttpClient : public Http {
		public:
			HttpClient(enet::Tcp _connection);
		public:
			void setHeader(const enet::HttpRequest& _header) {
				setRequestHeader(_header);
			}
		public:
			//bool get(const std::string& _address);
			//bool post(const std::string& _address, const std::map<std::string, std::string>& _values);
			//bool post(const std::string& _address, const std::string& _contentType, const std::string& _data);
		public:
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connectHeader(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(const enet::HttpAnswer&)) {
				m_observerAnswer = [=](const enet::HttpAnswer& _value){
					(*_class.*_func)(_value);
				};
			}
			void connectHeader(Http::ObserverAnswer _func) {
				m_observerAnswer = _func;
			}
	};
	
	class HttpServer : public Http {
		public:
			HttpServer(enet::Tcp _connection);
		public:
			void setHeader(const enet::HttpAnswer& _header) {
				setAnswerHeader(_header);
			}
		public:
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connectHeader(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(const enet::HttpRequest&)) {
				m_observerRequest = [=](const enet::HttpRequest& _value){
					(*_class.*_func)(_value);
				};
			}
			void connectHeader(Http::ObserverRequest _func) {
				m_observerRequest = _func;
			}
	};
}

