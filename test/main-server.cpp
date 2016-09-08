/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test-debug/debug.h>
#include <enet/Tcp.h>
#include <enet/Http.h>
#include <etk/etk.h>
#include <enet/TcpServer.h>

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
	//Wait on TCP connection:
	enet::TcpServer interface;
	// Configure server interface:
	interface.setHostNane("127.0.0.1");
	interface.setPort(31235);
	// Start listening ...
	interface.link();
	// Wait a new connection ..
	enet::Tcp tcpConnection = std::move(interface.waitNext());
	// Free Connected port
	interface.unlink();
	
	int32_t iii = 0;
	while (tcpConnection.getConnectionStatus() == enet::Tcp::status::link) {
		int32_t len = tcpConnection.write("plop" + etk::to_string(iii));
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
#else
	TEST_CRITICAL("not implemented");
#endif
	return 0;
}
