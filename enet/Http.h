/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <enet/Tcp.h>
#include <vector>
#include <map>
#include <thread>
#include <ethread/tools.h>

namespace enet {
	enum class HTTPAnswerCode {
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
	
	enum class HTTPProtocol {
		http_1_0,
		http_1_1,
	};
	class HttpHeader {
		private:
			// key, val
			std::map<std::string, std::string> m_map;
			enum HTTPProtocol m_protocol;
		public:
			void addKey(const std::string& _key, const std::string& _value);
			void rmKey(const std::string& _key);
			std::string getKey(const std::string& _key);
			enum HTTPProtocol getProtocol() {
				return m_protocol;
			}
			void setProtocol(enum HTTPProtocol _protocol) {
				m_protocol = _protocol;
			}
			virtual ~HttpHeader() = default;
	};
	class HttpAnswer : public HttpHeader {
		private:
			
			enet::HTTPAnswerCode m_what;
			int64_t m_messageSize; // parameter 
		public:
			HttpAnswer();
			HttpAnswer(const std::string& _value);
			get(const std::string& _uri);
			setSize(int64_t _messageSize=-1);
			void display() const;
			std::string generate();
	};
	enum class HTTPReqType {
		GET,
		HEAD,
		POST,
		PUT,
		DELETE,
	};
	class HttpRequest : public HttpHeader{
		private:
			// key, val
			std::map<std::string, std::string> m_parameters;
			enum HTTPReqType m_req;
			std::string m_uri;
			enum HTTPProtocol m_protocol;
			bool m_keepAlive;
		public:
			HttpRequest(enum HTTPReqType _type, 
			void display() const;
			std::string generate();
			
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
		private:
			enet::Tcp m_connection;
			bool m_headerIsSend;
			std::thread* m_thread;
			bool m_threadRunning;
			std::vector<uint8_t> m_temporaryBuffer;
		private:
			bool m_keepAlive;
			void threadCallback();
		public:
			void setKeepAlive(bool _keepAlive) {
				m_keepAlive = true;
			}
			bool getKeepAlive() {
				return m_keepAlive;
			}
		private:
			
			HttpHeader m_header;
			std::vector<uint8_t> m_receiveData;
			void getHeader();
		public:
			void start();
			void stop(bool _inThread=false);
			bool isAlive() {
				return m_connection.getConnectionStatus() == enet::Tcp::status::link;
			}
			void setSendHeaderProperties(const std::string& _key, const std::string& _val);
			std::string getSendHeaderProperties(const std::string& _key);
			std::string getReceiveHeaderProperties(const std::string& _key);
			bool get(const std::string& _address);
			bool post(const std::string& _address, const std::map<std::string, std::string>& _values);
			bool post(const std::string& _address, const std::string& _contentType, const std::string& _data);
			std::string dataString();
			void writeAnswerHeader(enum enet::HTTPAnswerCode _value);
			std::string escapeChar(const std::string& _value);
			std::string unEscapeChar(const std::string& _value);
		public:
			using Observer = std::function<void(enet::Http& _interface, std::vector<uint8_t>&)>; //!< Define an Observer: function pointer
			Observer m_observer;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(enet::Http& _interface, std::vector<uint8_t>&)) {
				m_observer = [=](enet::Http& _interface, std::vector<uint8_t>& _value){
					(*_class.*_func)(_interface,_value);
				};
			}
			void connect(Observer _func) {
				m_observer = _func;
			}
		public:
			using ObserverRequest = std::function<void(enet::Http& _interface, const enet::HttpHeader&)>; //!< Define an Observer: function pointer
			ObserverRequest m_observerRequest;
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connectHeader(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(enet::Http& _interface, const enet::HttpHeader&)) {
				m_observerRequest = [=](enet::Http& _interface, std::vector<uint8_t>& _value){
					(*_class.*_func)(_value);
				};
			}
			void connectHeader(ObserverRequest _func) {
				m_observerRequest = _func;
			}
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
}

