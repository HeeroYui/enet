/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <enet/Udp.hpp>
#include <enet/Tcp.hpp>
#include <enet/Http.hpp>
#include <enet/Ftp.hpp>


/**
 * @brief Main esvg namespace
 */
namespace enet {
	/**
	 * @brief Initialize enet
	 * @param[in] _argc Number of argument list
	 * @param[in] _argv List of arguments
	 */
	void init(int _argc, const char** _argv);
	/**
	 * @brief un-Initialize enet
	 */
	void unInit();
	/**
	 * @brief Check if the library is initialized
	 * @return bool value to chek if initialize ot not
	 */
	bool isInit();
}

