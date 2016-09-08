/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test-debug/debug.h>
#include <enet/Tcp.h>
#include <enet/TcpClient.h>
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
	TEST_INFO("== Test TCP client              ==");
	TEST_INFO("==================================");
#ifndef __TARGET_OS__Windows
	// client mode ...
	// connect on TCP server:
	enet::Tcp connection = std::move(enet::connectTcpClient("127.0.0.1", 12345));
	TEST_INFO("CLIENT connect ...");
	if (connection.getConnectionStatus() != enet::Tcp::status::link) {
		TEST_ERROR("can not link to the socket...");
		return -1;
	}
	int32_t iii = 0;
	while (    connection.getConnectionStatus() == enet::Tcp::status::link
	        && iii<10000) {
		char data[1024];
		int32_t len = connection.read(data, 1024);
		TEST_INFO("read len=" << len << " data='" << data << "'");
		//if (data[len-1] == '2') {
			int32_t lenWrite = connection.write("get pair value");
			TEST_INFO("write len=" << lenWrite);
		//}
		iii++;
	}
	if (iii>=10000) {
		TEST_INFO("auto disconnected");
	} else if (connection.getConnectionStatus() != enet::Tcp::status::link) {
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
