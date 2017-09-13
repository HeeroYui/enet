/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <test-debug/debug.hpp>
#include <enet/enet.hpp>
#include <enet/Tcp.hpp>
#include <enet/Http.hpp>
#include <enet/TcpServer.hpp>
#include <etk/etk.hpp>


#include <etk/stdTools.hpp>
namespace appl {
	void onReceiveData(enet::HttpServer* _interface, etk::Vector<uint8_t>& _data) {
		TEST_INFO("Receive Datas : " << _data.size() << " bytes");
	}
	void onReceiveHeader(enet::HttpServer* _interface, const enet::HttpRequest& _data) {
		TEST_INFO("Receive Header data:");
		_data.display();
		if (_data.getType() == enet::HTTPReqType::HTTP_GET) {
			if (_data.getUri() == "plop.txt") {
				enet::HttpAnswer answer(enet::HTTPAnswerCode::c200_ok);
				etk::String data = "<html><head></head></body>coucou</body></html>";
				answer.setKey("Content-Length", etk::toString(data.size()));
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
	TEST_INFO("== Test HTTP server             ==");
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
	// TODO : Check if connection is valid ...
	
	// Create a HTTP connection in Server mode
	enet::HttpServer connection(etk::move(tcpConnection));
	enet::HttpServer* tmp = &connection;
	// Set callbacks:
	connection.connect([=](etk::Vector<uint8_t>& _value){
					appl::onReceiveData(tmp, _value);
				});
	connection.connectHeader([=](const enet::HttpRequest& _value){
					appl::onReceiveHeader(tmp, _value);
				});
	
	// start http connection (the actual state is just TCP start ...)
	connection.start();
	
	while (connection.isAlive() == true) {
		ethread::sleepMilliSeconds((100));
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
	etk::Map<etk::String, etk::String> values;
	values.insert(etk::makePair<etk::String, etk::String>("plop", "valuePlop"));
	if (connection.post("", values) == false) {
		TEST_ERROR("can not POST data...");
		return -1;
	}
	TEST_INFO("data : " << connection.dataString());
	
	TEST_INFO("----------------------------");
	TEST_INFO("POST xml : ");
	if (connection.post("", "text/xml; charset=utf-8", "<plop><etk/String.hpp>value1</string></plop>") == false) {
		TEST_ERROR("can not POST XML data...");
		return -1;
	}
	TEST_INFO("data : " << connection.dataString());
	*/
	return 0;
}
