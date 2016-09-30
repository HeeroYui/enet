/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <enet/Tcp.hpp>

namespace enet {
	enet::Tcp connectTcpClient(uint8_t _ip1, uint8_t _ip2, uint8_t _ip3, uint8_t _ip4, uint16_t _port, uint32_t _numberRetry=5);
	enet::Tcp connectTcpClient(const std::string& _hostname, uint16_t _port, uint32_t _numberRetry=5);
}
