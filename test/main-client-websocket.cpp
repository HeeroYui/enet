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
#include <enet/WebSocket.hpp>
#include <etk/etk.hpp>

#include <etk/stdTools.hpp>


namespace appl {
	void onReceiveData(etk::Vector<uint8_t>& _data, bool _isString) {
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
	TEST_INFO("== Test WebSocket client        ==");
	TEST_INFO("==================================");
	// connect on TCP server:
	enet::Tcp tcpConnection = etk::move(enet::connectTcpClient("127.0.0.1", 12345));
	// TODO : Check if connection is valid ...
	
	// Create a HTTP connection in Client mode
	enet::WebSocket connection(etk::move(tcpConnection), false);
	// Set callbacks:
	connection.connect(appl::onReceiveData);
	
	// start http connection (the actual state is just TCP start ...)
	etk::Vector<etk::String> protocols;
	protocols.pushBack("test1526/1.0");
	protocols.pushBack("test1526/1.5");
	protocols.pushBack("Hello");
	connection.start("/plop.txt", protocols);
	
	// send some data to play ...
	connection.write("coucou comment ca vas ???");
	
	int32_t timeout = 20;
	while (connection.isAlive() == true
	        && timeout > 0) {
		ethread::sleepMilliSeconds((100));
		timeout--;
	}
	return 0;
}
