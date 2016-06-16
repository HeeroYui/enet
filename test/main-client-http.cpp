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
#include <unistd.h>

namespace appl {
	void onReceiveData(enet::Http& _interface, std::vector<uint8_t>& _data) {
		TEST_INFO("Receive Datas : " << _data.size() << " bytes");
		TEST_INFO("data:" << (char*)&_data[0] << "");
	}
}

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
	TEST_INFO("== Test HTTP client             ==");
	TEST_INFO("==================================");
#ifndef __TARGET_OS__Windows
	// connect on TCP server:
	enet::Tcp tcpConnection = std::move(enet::connectTcpClient("127.0.0.1", 12345));
	// TODO : Check if connection is valid ...
	
	// Create a HTTP connection in Client mode
	enet::Http connection(std::move(tcpConnection), false);
	connection.setKeepAlive(true);
	// Set callbacks:
	connection.connect(appl::onReceiveData);
	
	// start http connection (the actual state is just TCP start ...)
	connection.start();
	connection.get("plop.txt");
	
	
	while (connection.isAlive() == true) {
		usleep(100000);
	}
	
#else
	TEST_CRITICAL("not implemented");
#endif
	return 0;
}
