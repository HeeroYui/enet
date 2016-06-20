/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <enet/Http.h>
#include <ememory/memory.h>
#include <vector>
#include <map>

namespace enet {
	class WebSocket {
		protected:
			ememory::SharedPtr<enet::Http> m_interface;
			std::vector<uint8_t> m_buffer;
			std::string m_checkKey;
		public:
			WebSocket(enet::Tcp _connection, bool _isServer=false);
			virtual ~WebSocket();
			void start(const std::string& _uri="");
			void stop(bool _inThread=false);
			bool isAlive() {
				return m_interface->isAlive();
			}
			void onReceiveData(enet::Tcp& _data);
			void onReceiveRequest(const enet::HttpRequest& _data);
			void onReceiveAnswer(const enet::HttpAnswer& _data);
		public:
			using Observer = std::function<void(std::vector<uint8_t>&, bool)>; //!< Define an Observer: function pointer
		protected:
			Observer m_observer;
		public:
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(std::vector<uint8_t>&, bool)) {
				m_observer = [=](std::vector<uint8_t>& _value, bool _isString){
					(*_class.*_func)(_value, _isString);
				};
			}
			void connect(Observer _func) {
				m_observer = _func;
			}
		// Only server:
		public:
			using ObserverUriCheck = std::function<bool(const std::string&)>; //!< Define an Observer: function pointer
		protected:
			ObserverUriCheck m_observerUriCheck;
		public:
			/**
			 * @brief Connect an function member on the signal with the shared_ptr object.
			 * @param[in] _class shared_ptr Object on whe we need to call ==> the object is get in keeped in weak_ptr.
			 * @param[in] _func Function to call.
			 * @param[in] _args Argument optinnal the user want to add.
			 */
			template<class CLASS_TYPE>
			void connectUri(CLASS_TYPE* _class, bool (CLASS_TYPE::*_func)(const std::string&)) {
				m_observerUriCheck = [=](const std::string& _value){
					(*_class.*_func)(_value);
				};
			}
			void connectUri(ObserverUriCheck _func) {
				m_observerUriCheck = _func;
			}
		public:
			/**
			 * @brief Write a chunk of data on the socket
			 * @param[in] _data pointer on the data might be write
			 * @param[in] _len Size that must be written socket
			 * @return >0 byte size on the socket write
			 * @return -1 an error occured.
			 */
			//TODO : ...
			int32_t write(const void* _data, int32_t _len, bool _isString=false, bool _mask= false);
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
					return write(_data.c_str(), _data.size()+1, true);
				}
				return write(_data.c_str(), _data.size(), true);
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
		protected:
			void controlPing();
			void controlPong();
			void contolClose();
	};
}
