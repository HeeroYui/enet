/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test-debug/debug.hpp>
#include <enet/Tcp.hpp>
#include <enet/Http.hpp>
#include <enet/WebSocket.hpp>
#include <enet/TcpServer.hpp>
#include <etk/etk.hpp>

#include <unistd.h>
#include <etk/stdTools.hpp>
namespace appl {
	void onReceiveData(enet::WebSocket* _interface, std::vector<uint8_t>& _data, bool _isString) {
		TEST_INFO("Receive Datas : " << _data.size() << " bytes");
		if (_isString == true) {
			_data.resize(_data.size()+1);
			_data[_data.size()-1] = '\0';
			TEST_INFO("string data: '" << (char*)&_data[0] << "'");
			_interface->write("Très bien, merci ...");
		} else {
			TEST_INFO("binary data: ... ");
		}
	}
	bool onReceiveUri(enet::WebSocket* _interface, const std::string& _uri, const std::vector<std::string>& _protocols) {
		TEST_INFO("Receive Header uri: " << _uri);
		for (auto &it : _protocols) {
			if (it == "test1526/1.5") {
				_interface->setProtocol(it);
				break;
			}
		}
		if (_uri == "/plop.txt") {
			return true;
		}
		return false;
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
	TEST_INFO("== Test WebSocket server        ==");
	TEST_INFO("==================================");
#ifndef __TARGET_OS__Windows
	//Wait on TCP connection:
	enet::TcpServer interface;
	// Configure server interface:
	interface.setHostNane("127.0.0.1");
	interface.setPort(12345);
	// Start listening ...
	interface.link();
	// Wait a new connection ..
	enet::Tcp tcpConnection = std::move(interface.waitNext());
	// Free Connected port
	interface.unlink();
	// TODO : Check if connection is valid ...
	
	// Create a HTTP connection in Server mode
	enet::WebSocket connection(std::move(tcpConnection), true);
	enet::WebSocket* tmp = &connection;
	// Set callbacks:
	connection.connect([=](std::vector<uint8_t>& _value, bool _isString){
					appl::onReceiveData(tmp, _value, _isString);
				});
	connection.connectUri([=](const std::string& _value, const std::vector<std::string>& _protocols){
					return appl::onReceiveUri(tmp, _value, _protocols);
				});
	
	// start http connection (the actual state is just TCP start ...)
	connection.start();
	
	while (connection.isAlive() == true) {
		usleep(100000);
	}
#else
	TEST_CRITICAL("not implemented");
#endif
	return 0;
}
