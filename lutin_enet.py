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
	return "authors.txt"

def get_version():
	return "version.txt"

def create(target, module_name):
	my_module = module.Module(__file__, module_name, get_type())
	my_module.add_depend([
	    'etk',
	    'ememory',
	    'algue'
	    ])
	my_module.add_path(tools.get_current_path(__file__))
	my_module.add_src_file([
	    'enet/debug.cpp',
	    'enet/Udp.cpp',
	    'enet/Tcp.cpp',
	    'enet/TcpServer.cpp',
	    'enet/TcpClient.cpp',
	    'enet/Http.cpp',
	    'enet/Ftp.cpp',
	    'enet/WebSocket.cpp',
	    ])
	my_module.add_header_file([
	    'enet/debug.hpp',
	    'enet/Udp.hpp',
	    'enet/Tcp.hpp',
	    'enet/TcpServer.hpp',
	    'enet/TcpClient.hpp',
	    'enet/Http.hpp',
	    'enet/Ftp.hpp',
	    'enet/WebSocket.hpp',
	    ])
	return my_module







