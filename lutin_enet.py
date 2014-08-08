#!/usr/bin/python
import lutinModule as module
import lutinTools as tools

def get_desc():
	return "e-net TCP/UDP/HTTP/FTP interface"

def get_licence():
	return {
		"assimilate":"APACHE2",
		"type":"APACHE-2.0"
		}

def create(target):
	myModule = module.Module(__file__, 'enet', 'LIBRARY')
	
	myModule.add_module_depend(['etk'])
	
	myModule.add_src_file([
		'enet/debug.cpp',
		'enet/Udp.cpp',
		'enet/Tcp.cpp',
		'enet/Http.cpp',
		'enet/Ftp.cpp',
		])
	
	myModule.add_export_path(tools.get_current_path(__file__))
	
	# add the currrent module at the 
	return myModule







