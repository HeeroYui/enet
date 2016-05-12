/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

namespace enet {
	class Tcp {
		private:
			int32_t m_socketId; //!< socket linux interface generic
			int32_t m_socketIdClient;
		public:
			Tcp();
			virtual ~Tcp();
		private:
			std::string m_host; //!< hostname/IP to connect with.
		public:
			/**
			 * @brief Set the connection IP id.
			 * @param[in] _first Firt number of the IP v4.
			 * @param[in] _second Second number of the IP v4.
			 * @param[in] _third Third number of the IP v4.
			 * @param[in] _quatro Quatro number of the IP v4.
			 */
			void setIpV4(uint8_t _fist, uint8_t _second, uint8_t _third, uint8_t _quatro);
			/**
			 * @brief set the Host name is the same things as set an Ip adress, but in test mode "127.0.0.1" or "localhost".
			 * @param[in] _name Host name to connect.
			 */
			void setHostNane(const std::string& _name);
			/**
			 * @brief Get the decriptive name hot the host
			 * @return the string requested
			 */
			const std::string& getHostName() {
				return m_host;
			}
		private:
			uint16_t m_port; //!< IP port to connect with.
		public:
			/**
			 * @brief set the port number to connect or to spy
			 * @param[in] _port Number of the port requested
			 */
			void setPort(uint16_t _port);
			/**
			 * @brief Get the port number.
			 * @return The requested port number.
			 */
			uint16_t getPort() {
				return m_port;
			}
		private:
			bool m_server; //!< if at true, the server mode is requested
		public:
			/**
			 * @brief Set the TCP interface in server mode
			 * @param[in] _status if true, this enable the server mode
			 */
			void setServer(bool _status);
			/**
			 * @brief Get the server mode status.
			 * @return true: the tcp interface is configure as a server.
			 */
			int32_t getServer() {
				return m_server;
			}
		public:
			enum class status {
				unlink,
				link,
				error
			};
		private:
			enum status m_status; //!< current connection status
		public:
			/**
			 * @brief Get the current Status of the connection
			 * @return The status.
			 */
			enum status getConnectionStatus() {
				return m_status;
			}
		public:
			/**
			 * @brief Link on a specific interface.
			 * @return true if connection is done
			 * @return false otherwise ...
			 */
			bool link();
			/**
			 * @brief Unlink on a specific interface.
			 * @return true if connection is removed
			 * @return false otherwise ...
			 */
			bool unlink();
			/**
			 * @brief Read a chunk of data on the socket
			 * @param[in] _data pointer on the data might be write
			 * @param[in] _maxLen Size that can be written on the pointer
			 * @return >0 byte size on the socket read
			 * @return -1 an error occured.
			 */
			int32_t read(void* _data, int32_t _maxLen);
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

