/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <enet/Tcp.h>
#include <vector>
#include <map>

namespace enet {
	class WebSocket{
		private:
			enet::Http m_interface;
		public:
			WebSocket(enet::Tcp _connection, bool _isServer=false);
			virtual ~WebSocket();
			void start(const std::string& _uri);
			void stop(bool _inThread=false);
	};
}
