/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <enet/Tcp.h>
#ifdef __TARGET_OS__Windows
	
#else
	#include <poll.h>
#endif

namespace enet {
	class TcpServer {
		private:
			int32_t m_socketId; //!< socket linux interface generic
			#ifndef __TARGET_OS__Windows
				struct pollfd m_fds[1];
			#endif
		public:
			TcpServer();
			virtual ~TcpServer();
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
		public:
			bool link();
			bool unlink();
			/**
			 * @brief Wait next extern connection
			 * @return element with the connection
			 */
			enet::Tcp waitNext();
	};
}

