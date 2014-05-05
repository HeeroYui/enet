/**
 * @author Edouard DUPIN
 * 
 * @copyright 2014, Edouard DUPIN, all right reserved
 * 
 * @license BSD 3 clauses (see license file)
 */

#ifndef __ENET_HTTP_H__
#define __ENET_HTTP_H__

#include <enet/Tcp.h>
#include <vector>
#include <map>


namespace enet {
	class Http {
		public:
			Http(void);
			virtual ~Http(void);
		private:
			enet::Tcp m_connection;
		private:
			// key, val
			std::map<std::string, std::string> m_sendHeader;
			std::map<std::string, std::string> m_receiveHeader;
			std::vector<uint8_t> m_receiveData;
			bool connect(void);
			bool reset(void);
		public:
			void setSendHeaderProperties(const std::string& _key, const std::string& _val);
			std::string getSendHeaderProperties(const std::string& _key);
			std::string getReceiveHeaderProperties(const std::string& _key);
			bool setServer(const std::string& _hostName);
			bool setPort(uint16_t _port);
			bool get(const std::string& _address);
			bool post(const std::string& _address, const std::map<std::string, std::string>& _values);
			
			int32_t dataSize(void) {
				return m_receiveData.size();
			}
			const std::vector<uint8_t>& data(void) {
				return m_receiveData;
			}
			std::string dataString(void);
	};
};

#endif
