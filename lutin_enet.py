#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools

def get_desc():
	return "e-net TCP/UDP/HTTP/FTP interface"

def get_licence():
	return {
		"assimilate":"APACHE2",
		"type":"APACHE-2.0"
		}

def create(target):
	my_module = module.Module(__file__, 'enet', 'LIBRARY')
	
	my_module.add_module_depend(['etk'])
	
	my_module.add_src_file([
		'enet/debug.cpp',
		'enet/Udp.cpp',
		'enet/Tcp.cpp',
		'enet/Http.cpp',
		'enet/Ftp.cpp',
		])
	
	my_module.add_export_path(tools.get_current_path(__file__))
	
	# add the currrent module at the 
	return my_module







