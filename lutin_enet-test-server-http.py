#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools


def get_type():
	return "BINARY"

def get_sub_type():
	return "TEST"

def get_desc():
	return "e-net TEST test software for enet"

def get_licence():
	return "APACHE-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def create(target, module_name):
	my_module = module.Module(__file__, module_name, get_type())
	my_module.add_path(tools.get_current_path(__file__), export=True)
	my_module.add_depend(['enet', 'gtest', 'test-debug'])
	my_module.add_src_file([
	    'test/main-server-http.cpp'
	    ])
	return my_module







