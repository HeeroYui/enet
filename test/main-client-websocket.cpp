/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test-debug/debug.h>
#include <enet/Tcp.h>
#include <enet/TcpClient.h>
#include <enet/Http.h>
#include <enet/WebSocket.h>
#include <etk/etk.h>

#include <etk/stdTools.h>
#include <unistd.h>

namespace appl {
	void onReceiveData(std::vector<uint8_t>& _data, bool _isString) {
		TEST_INFO("Receive Datas : " << _data.size() << " bytes");
		if (_isString == true) {
			_data.resize(_data.size()+1);
			_data[_data.size()-1] = '\0';
			TEST_INFO("string data: '" << (char*)&_data[0] << "'");
		} else {
			TEST_INFO("binary data: ... ");
		}
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
	TEST_INFO("== Test WebSocket client        ==");
	TEST_INFO("==================================");
#ifndef __TARGET_OS__Windows
	// connect on TCP server:
	enet::Tcp tcpConnection = std::move(enet::connectTcpClient("127.0.0.1", 12345));
	// TODO : Check if connection is valid ...
	
	// Create a HTTP connection in Client mode
	enet::WebSocket connection(std::move(tcpConnection), false);
	// Set callbacks:
	connection.connect(appl::onReceiveData);
	
	// start http connection (the actual state is just TCP start ...)
	std::vector<std::string> protocols;
	protocols.push_back("test1526/1.0");
	protocols.push_back("test1526/1.5");
	protocols.push_back("Hello");
	connection.start("/plop.txt", protocols);
	
	// send some data to play ...
	connection.write("coucou comment ca vas ???");
	
	int32_t timeout = 20;
	while (connection.isAlive() == true
	        && timeout > 0) {
		usleep(100000);
		timeout--;
	}
	
#else
	TEST_CRITICAL("not implemented");
#endif
	return 0;
}
