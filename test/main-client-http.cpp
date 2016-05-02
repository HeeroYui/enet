/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test-debug/debug.h>
#include <enet/Tcp.h>
#include <enet/Http.h>
#include <etk/etk.h>

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
	TEST_INFO("== Test HTTP client             ==");
	TEST_INFO("==================================");
#ifndef __TARGET_OS__Windows
	// client mode ...
	enet::Http connection;
	connection.setServer("127.0.0.1");
	connection.setKeepAlive(true);
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
	if (connection.post("", /*"application/xml"*/ "text/xml; charset=utf-8", "<plop><string>value1</string></plop>") == false) {
		TEST_ERROR("can not POST XML data...");
		return -1;
	}
	TEST_INFO("data : " << connection.dataString());
#else
	TEST_CRITICAL("not implemented");
#endif
	return 0;
}
