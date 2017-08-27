/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <enet/Http.hpp>
#include <ememory/memory.hpp>
#include <echrono/Steady.hpp>
#include <etk/Vector.hpp>
#include <etk/Map.hpp>

namespace enet {
	class WebSocket {
		protected:
			etk::Vector<uint8_t> m_sendBuffer;
			bool m_connectionValidate;
			ememory::SharedPtr<enet::Http> m_interface;
			etk::Vector<uint8_t> m_buffer;
			etk::String m_checkKey;
			echrono::Steady m_lastReceive;
			echrono::Steady m_lastSend;
			std::mutex m_mutex;
		public:
			const echrono::Steady& getLastTimeReceive() {
				return m_lastReceive;
			}
			const echrono::Steady& getLastTimeSend() {
				return m_lastSend;
			}
		public:
			WebSocket();
			WebSocket(enet::Tcp _connection, bool _isServer=false);
			void setInterface(enet::Tcp _connection, bool _isServer=false);
			virtual ~WebSocket();
			void start(const etk::String& _uri="", const etk::Vector<etk::String>& _listProtocols=etk::Vector<etk::String>());
			void stop(bool _inThread=false);
			bool isAlive() const {
				if (m_interface == nullptr) {
					return false;
				}
				return m_interface->isAlive();
			}
			void onReceiveData(enet::Tcp& _data);
			void onReceiveRequest(const enet::HttpRequest& _data);
			void onReceiveAnswer(const enet::HttpAnswer& _data);
		protected:
			etk::String m_protocol;
		public:
			void setProtocol(const etk::String& _protocol) {
				m_protocol = _protocol;
			}
		public:
			using Observer = std::function<void(etk::Vector<uint8_t>&, bool)>; //!< Define an Observer: function pointer
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
			void connect(CLASS_TYPE* _class, void (CLASS_TYPE::*_func)(etk::Vector<uint8_t>&, bool)) {
				m_observer = [=](etk::Vector<uint8_t>& _value, bool _isString){
					(*_class.*_func)(_value, _isString);
				};
			}
			void connect(Observer _func) {
				m_observer = _func;
			}
		// Only server:
		public:
			using ObserverUriCheck = std::function<bool(const etk::String&, const etk::Vector<etk::String>&)>; //!< Define an Observer: function pointer
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
			void connectUri(CLASS_TYPE* _class, bool (CLASS_TYPE::*_func)(const etk::String&, const etk::Vector<etk::String>&)) {
				m_observerUriCheck = [=](const etk::String& _value, const etk::Vector<etk::String>& _protocols){
					return (*_class.*_func)(_value, _protocols);
				};
			}
			void connectUri(ObserverUriCheck _func) {
				m_observerUriCheck = _func;
			}
		private:
			bool m_isString;
			bool m_haveMask;
			uint8_t m_dataMask[4];
		public:
			std::unique_lock<std::mutex> getScopeLock() {
				return etk::move(std::unique_lock<std::mutex>(m_mutex));
			}
			/**
			 * Compose the local header inside a temporary buffer ==> must lock external to prevent multiple simultaneous access
			 */
			bool configHeader(bool _isString=false, bool _mask= false);
			/**
			 * CWrite data in a temporary buffer ==> must lock external to prevent multiple simultaneous access
			 */
			int32_t writeData(uint8_t* _data, int32_t _len);
			/**
			 * Use temporary buffer to send it in the socket ==> must lock external to prevent multiple simultaneous access
			 */
			int32_t send();
			/**
			 * @brief Write a chunk of data on the socket
			 * @param[in] _data pointer on the data might be write
			 * @param[in] _len Size that must be written socket
			 * @return >0 byte size on the socket write
			 * @return -1 an error occured.
			 * @note: This function is locked ...
			 */
			int32_t write(const void* _data, int32_t _len, bool _isString=false, bool _mask= false);
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
		public:
			void controlPing();
			void controlPong();
			void controlClose();
	};
}
