#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools


def get_type():
	return "LIBRARY"

def get_desc():
	return "TCP/UDP/HTTP/FTP interface"

def get_licence():
	return "APACHE-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def get_version():
	return [0,2,0]

def create(target, module_name):
	my_module = module.Module(__file__, module_name, get_type())
	my_module.add_module_depend(['etk', 'ememory', 'algue'])
	my_module.add_src_file([
	    'enet/debug.cpp'
	    ])
	my_module.add_path(tools.get_current_path(__file__))
	if target.name == "Windows":
		return my_module
	my_module.add_src_file([
	    'enet/Udp.cpp',
	    'enet/Tcp.cpp',
	    'enet/TcpServer.cpp',
	    'enet/TcpClient.cpp',
	    'enet/Http.cpp',
	    'enet/Ftp.cpp',
	    'enet/WebSocket.cpp',
	    ])
	my_module.add_header_file([
	    'enet/debug.h',
	    'enet/Udp.h',
	    'enet/Tcp.h',
	    'enet/TcpServer.h',
	    'enet/TcpClient.h',
	    'enet/Http.h',
	    'enet/Ftp.h',
	    'enet/WebSocket.h',
	    ])
	return my_module







