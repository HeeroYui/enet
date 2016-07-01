/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <etk/types.h>
#include <poll.h>
#include <mutex>

namespace enet {
	class Tcp {
		private:
			int32_t m_socketId; //!< socket linux interface generic
			struct pollfd m_fds[1];
			std::mutex m_mutex;
		public:
			Tcp();
			Tcp(int32_t _idSocket, const std::string& _name);
			// move constructor
			Tcp(Tcp&& _obj);
			// Move operator;
			Tcp& operator= (Tcp&& _obj);
			// Remove copy operator ... ==> not valid ...
			Tcp& operator= (Tcp& _obj) = delete;
			virtual ~Tcp();
		private:
			std::string m_name; //!< hostname/IP:port.
		public:
			/**
			 * @brief Get the decriptive name hot the host:port
			 * @return the string requested
			 */
			const std::string& getName() {
				return m_name;
			}
		public:
			enum class status {
				unlink,
				link,
				linkRemoteClose,
				error
			};
		private:
			enum status m_status; //!< current connection status
		public:
			/**
			 * @brief Get the current Status of the connection
			 * @return The status.
			 */
			enum status getConnectionStatus() const {
				return m_status;
			}
		public:
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
			
			bool setTCPNoDelay(bool _enabled);
	};
}

