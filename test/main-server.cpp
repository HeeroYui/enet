/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <test-debug/debug.hpp>
#include <enet/enet.hpp>
#include <enet/Tcp.hpp>
#include <enet/Http.hpp>
#include <etk/etk.hpp>
#include <enet/TcpServer.hpp>

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
	TEST_INFO("== Test TCP server              ==");
	TEST_INFO("==================================");
	//Wait on TCP connection:
	enet::TcpServer interface;
	// Configure server interface:
	interface.setHostNane("127.0.0.1");
	interface.setPort(12345);
	// Start listening ...
	interface.link();
	// Wait a new connection ..
	enet::Tcp tcpConnection = etk::move(interface.waitNext());
	// Free Connected port
	interface.unlink();
	
	int32_t iii = 0;
	while (tcpConnection.getConnectionStatus() == enet::Tcp::status::link) {
		int32_t len = tcpConnection.write("plop" + etk::toString(iii));
		TEST_INFO("write len=" << len);
		char data[1024];
		len = tcpConnection.read(data, 1024);
		if (len > 0) {
			TEST_INFO("read len=" << len << " data='" << data << "'");
		}
		iii++;
	}
	if (iii>=1000000) {
		TEST_INFO("auto disconnected");
	} else if (tcpConnection.getConnectionStatus() != enet::Tcp::status::link) {
		TEST_INFO("server disconnected");
	} else {
		TEST_INFO("ERROR disconnected");
	}
	if (tcpConnection.unlink() == false) {
		TEST_ERROR("can not unlink to the socket...");
		return -1;
	}
	return 0;
}
