/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test-debug/debug.h>
#include <enet/Tcp.h>
#include <enet/Http.h>
#include <enet/TcpServer.h>
#include <etk/etk.h>

#include <unistd.h>
#include <etk/stdTools.h>
namespace appl {
	void onReceiveData(enet::HttpServer* _interface, std::vector<uint8_t>& _data) {
		TEST_INFO("Receive Datas : " << _data.size() << " bytes");
	}
	void onReceiveHeader(enet::HttpServer* _interface, const enet::HttpRequest& _data) {
		TEST_INFO("Receive Header data:");
		_data.display();
		if (_data.getType() == enet::HTTPReqType::GET) {
			if (_data.getUri() == "plop.txt") {
				enet::HttpAnswer answer(enet::HTTPAnswerCode::c200_ok);
				std::string data = "<html><head></head></body>coucou</body></html>";
				answer.setKey("Content-Length", etk::to_string(data.size()));
				_interface->setHeader(answer);
				_interface->write(data);
				_interface->stop(true);
				return;
			}
		}
		enet::HttpAnswer answer(enet::HTTPAnswerCode::c404_notFound);
		answer.setKey("Connection", "close");
		_interface->setHeader(answer);
		_interface->stop(true);
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
	TEST_INFO("== Test HTTP server             ==");
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
	enet::HttpServer connection(std::move(tcpConnection));
	enet::HttpServer* tmp = &connection;
	// Set callbacks:
	connection.connect([=](std::vector<uint8_t>& _value){
					appl::onReceiveData(tmp, _value);
				});
	connection.connectHeader([=](const enet::HttpRequest& _value){
					appl::onReceiveHeader(tmp, _value);
				});
	
	// start http connection (the actual state is just TCP start ...)
	connection.start();
	
	while (connection.isAlive() == true) {
		usleep(100000);
	}
	
	
	/*
	TEST_INFO("----------------------------");
	TEST_INFO("GET data : ");
	if (connection.get("") == false) {
		TEST_ERROR("can not GET data...");
		return -1;
	}
	TEST_INFO("data : " << connection.dataString());
	
	TEST_INFO("----------------------------");
	TEST_INFO("POST data : ");
	std::map<std::string, std::string> values;
	values.insert(std::make_pair<std::string, std::string>("plop", "valuePlop"));
	if (connection.post("", values) == false) {
		TEST_ERROR("can not POST data...");
		return -1;
	}
	TEST_INFO("data : " << connection.dataString());
	
	TEST_INFO("----------------------------");
	TEST_INFO("POST xml : ");
	if (connection.post("", "text/xml; charset=utf-8", "<plop><string>value1</string></plop>") == false) {
		TEST_ERROR("can not POST XML data...");
		return -1;
	}
	TEST_INFO("data : " << connection.dataString());
	*/
#else
	TEST_CRITICAL("not implemented");
#endif
	return 0;
}
