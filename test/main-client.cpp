/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <test-debug/debug.hpp>
#include <enet/enet.hpp>
#include <enet/Tcp.hpp>
#include <enet/TcpClient.hpp>
#include <enet/Http.hpp>
#include <etk/etk.hpp>
#include <iostream>
#include <etk/stdTools.hpp>

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	enet::init(_argc, _argv);
	for (int32_t iii=0; iii<_argc ; ++iii) {
		etk::String data = _argv[iii];
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
	// client mode ...
	// connect on TCP server:
	enet::Tcp connection = etk::move(enet::connectTcpClient("127.0.0.1", 12345));
	TEST_INFO("CLIENT connect ...");
	if (connection.getConnectionStatus() != enet::Tcp::status::link) {
		TEST_ERROR("can not link to the socket...");
		return -1;
	}
	int32_t iii = 0;
	int32_t delay = 200;
	while (    connection.getConnectionStatus() == enet::Tcp::status::link
	        && iii<10000) {
		char data[1024];
		int32_t len = connection.read(data, 1024);
		TEST_INFO("read len=" << len << " data='" << data << "'");
		//if (data[len-1] == '2') {
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			delay--;
			if (delay == 0) {
				delay = 500;
			}
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
	return 0;
}
