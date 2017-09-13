/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/types.hpp>
#include <ethread/Mutex.hpp>
#include <etk/Function.hpp>
#ifdef __TARGET_OS__Windows
	#include <winsock2.h>
	#include <ws2tcpip.h>
#endif

//#define ENET_STORE_INPUT

#ifdef ENET_STORE_INPUT
	#include <etk/os/FSNode.hpp>
#endif

namespace enet {
	class Tcp {
		private:
			#ifdef __TARGET_OS__Windows
				SOCKET m_socketId; //!< socket Windows interface generic
			#else
				int32_t m_socketId; //!< socket linux interface generic
			#endif
			#ifdef ENET_STORE_INPUT
				etk::FSNode m_nodeStoreInput;
			#endif
			ethread::Mutex m_mutex;
		public:
			Tcp();
			#ifdef __TARGET_OS__Windows
				Tcp(SOCKET _idSocket, const etk::String& _name);
			#else
				Tcp(int32_t _idSocket, const etk::String& _name);
			#endif
			// move constructor
			Tcp(Tcp&& _obj);
			// Move operator;
			Tcp& operator= (Tcp&& _obj);
			// Remove copy operator ... ==> not valid ...
			Tcp& operator= (Tcp& _obj) = delete;
			virtual ~Tcp();
		private:
			etk::String m_name; //!< hostname/IP:port.
		public:
			/**
			 * @brief Get the decriptive name hot the host:port
			 * @return the string requested
			 */
			const etk::String& getName() {
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
			int32_t write(const etk::String& _data, bool _writeBackSlashZero = true) {
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
			int32_t write(const etk::Vector<T>& _data) {
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

