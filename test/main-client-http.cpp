/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test-debug/debug.hpp>
#include <enet/enet.hpp>
#include <enet/Tcp.hpp>
#include <enet/TcpClient.hpp>
#include <enet/Http.hpp>
#include <etk/etk.hpp>

#include <etk/stdTools.hpp>
#include <unistd.h>

namespace appl {
	void onReceiveData(std::vector<uint8_t>& _data) {
		TEST_INFO("Receive Datas : " << _data.size() << " bytes");
		TEST_INFO("data:" << (char*)&_data[0] << "");
	}
}

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	enet::init(_argc, _argv);
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
	// connect on TCP server:
	enet::Tcp tcpConnection = std::move(enet::connectTcpClient("127.0.0.1", 12345));
	// TODO : Check if connection is valid ...
	
	// Create a HTTP connection in Client mode
	enet::HttpClient connection(std::move(tcpConnection));
	// Set callbacks:
	connection.connect(appl::onReceiveData);
	
	// start http connection (the actual state is just TCP start ...)
	connection.start();
	enet::HttpRequest req(enet::HTTPReqType::HTTP_GET);
	req.setUri("plop.txt");
	connection.setHeader(req);
	
	while (connection.isAlive() == true) {
		usleep(100000);
	}
	return 0;
}
