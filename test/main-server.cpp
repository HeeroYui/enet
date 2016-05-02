/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test-debug/debug.h>
#include <enet/Tcp.h>
#include <enet/Http.h>
#include <etk/etk.h>

#include <etk/stdTools.h>

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (    data == "-h"
		     || data == "--help") {
			TEST_PRINT(etk::getApplicationName() << " - help : ");
			TEST_PRINT("    " << _argv[0] << " [options]");
			TEST_PRINT("        No options ...");
			return -1;
		}
	}
	TEST_INFO("==================================");
	TEST_INFO("== Test TCP server              ==");
	TEST_INFO("==================================");
#ifndef __TARGET_OS__Windows
	// server mode ...
	enet::Tcp connection;
	connection.setHostNane("127.0.0.1");
	connection.setPort(31234);
	connection.setServer(true);
	TEST_INFO("SERVER connect ...");
	if (connection.link() == false) {
		TEST_ERROR("can not link to the socket...");
		return -1;
	}
	int32_t iii = 0;
	while (connection.getConnectionStatus() == enet::Tcp::statusLink) {
		int32_t len = connection.write("plop" + etk::to_string(iii));
		TEST_INFO("write len=" << len);
		iii++;
	}
	if (iii>=1000000) {
		TEST_INFO("auto disconnected");
	} else if (connection.getConnectionStatus() != enet::Tcp::statusLink) {
		TEST_INFO("server disconnected");
	} else {
		TEST_INFO("ERROR disconnected");
	}
	if (connection.unlink() == false) {
		TEST_ERROR("can not unlink to the socket...");
		return -1;
	}
#else
	TEST_CRITICAL("not implemented");
#endif
	return 0;
}
