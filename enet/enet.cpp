/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <enet/enet.hpp>
#include <enet/debug.hpp>

static bool& getInitSatatus() {
	static bool isInit = false;
	return isInit;
}
#ifdef __TARGET_OS__Windows
	#include <winsock2.h>
	#pragma comment(lib, "ws2_32.lib")
	
	WSADATA wsaData;
#endif

void enet::init(int _argc, const char** _argv) {
	for (int32_t iii=0; iii<_argc; ++iii) {
		std::string value = _argv[iii];
		if (etk::start_with(value, "--enet") == true) {
			ENET_ERROR("Unknow parameter type: '" << value << "'");
		}
	}
	if (getInitSatatus() == false) {
		#ifdef __TARGET_OS__Windows
			// Initialize Winsock
			int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
			if (iResult != 0) {
				ENET_ERROR("WSAStartup failed with error: " << iResult);
			}
		#endif
		getInitSatatus() = true;
	}
}

void enet::unInit() {
	if (getInitSatatus() == false) {
		ENET_ERROR("Request UnInit of enent already done ...");
	} else {
		#ifdef __TARGET_OS__Windows
			WSACleanup();
		#endif
	}
	getInitSatatus() = false;
}


bool enet::isInit() {
	return getInitSatatus();
}

