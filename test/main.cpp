/**
 * @author Edouard DUPIN
 * 
 * @copyright 2014, Edouard DUPIN, all right reserved
 * 
 * @license APACHE v2.0 (see license file)
 */

#include <test/debug.h>
#include <enet/Tcp.h>
#include <enet/Http.h>


#undef __class__
#define __class__ "test"

int main(int argc, const char *argv[]) {
	etk::log::setLevel(etk::log::logLevelDebug);
	APPL_VERBOSE("plop");
	if (argc > 2) {
		// client mode ...
		enet::Http connection;
		connection.setServer("127.0.0.1");
		connection.setKeepAlive(true);
		APPL_INFO("----------------------------");
		APPL_INFO("GET data : ");
		if (connection.get("") == false) {
			APPL_ERROR("can not GET data...");
			return -1;
		}
		APPL_INFO("data : " << connection.dataString());
		
		APPL_INFO("----------------------------");
		APPL_INFO("POST data : ");
		std::map<std::string, std::string> values;
		values.insert(std::make_pair<std::string, std::string>("plop", "valuePlop"));
		if (connection.post("", values) == false) {
			APPL_ERROR("can not POST data...");
			return -1;
		}
		APPL_INFO("data : " << connection.dataString());
		
		APPL_INFO("----------------------------");
		APPL_INFO("POST xml : ");
		if (connection.post("", /*"application/xml"*/ "text/xml; charset=utf-8", "<plop><string>value1</string></plop>") == false) {
			APPL_ERROR("can not POST XML data...");
			return -1;
		}
		APPL_INFO("data : " << connection.dataString());
		
	} else if (argc > 1) {
		// client mode ...
		enet::Tcp connection;
		connection.setHostNane("127.0.0.1");
		connection.setPort(31234);
		connection.setServer(false);
		APPL_INFO("CLIENT connect ...");
		if (connection.link() == false) {
			APPL_ERROR("can not link to the socket...");
			return -1;
		}
		int32_t iii = 0;
		while (    connection.getConnectionStatus() == enet::Tcp::statusLink
		        && iii<10000) {
			char data[1024];
			int32_t len = connection.read(data, 1024);
			APPL_INFO("read len=" << len << " data='" << data << "'");
			iii++;
		}
		if (iii>=10000) {
			APPL_INFO("auto disconnected");
		} else if (connection.getConnectionStatus() != enet::Tcp::statusLink) {
			APPL_INFO("server disconnected");
		} else {
			APPL_INFO("ERROR disconnected");
		}
		if (connection.unlink() == false) {
			APPL_ERROR("can not unlink to the socket...");
			return -1;
		}
	} else {
		// server mode ...
		enet::Tcp connection;
		connection.setHostNane("127.0.0.1");
		connection.setPort(31234);
		connection.setServer(true);
		APPL_INFO("SERVER connect ...");
		if (connection.link() == false) {
			APPL_ERROR("can not link to the socket...");
			return -1;
		}
		int32_t iii = 0;
		while (connection.getConnectionStatus() == enet::Tcp::statusLink) {
			int32_t len = connection.write("plop" + std::to_string(iii));
			APPL_INFO("write len=" << len);
			iii++;
		}
		if (iii>=1000000) {
			APPL_INFO("auto disconnected");
		} else if (connection.getConnectionStatus() != enet::Tcp::statusLink) {
			APPL_INFO("server disconnected");
		} else {
			APPL_INFO("ERROR disconnected");
		}
		if (connection.unlink() == false) {
			APPL_ERROR("can not unlink to the socket...");
			return -1;
		}
	}
	
	return 0;
}
