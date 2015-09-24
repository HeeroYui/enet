#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools

def get_desc():
	return "e-net TEST test software for enet"

def get_licence():
	return {
		"assimilate":"APACHE2",
		"type":"APACHE-2.0"
		}

def create(target):
	my_module = module.Module(__file__, 'enet-test', 'BINARY')
	
	my_module.add_module_depend(['enet', 'gtest'])
	
	my_module.add_src_file([
		'test/debug.cpp',
		'test/main.cpp'
		])
	
	my_module.add_export_path(tools.get_current_path(__file__))
	
	# add the currrent module at the 
	return my_module







