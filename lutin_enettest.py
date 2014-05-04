#!/usr/bin/python
import lutinModule as module
import lutinTools as tools

def get_desc():
	return "e-net TEST test software for enet"

def get_licence():
	return {
		"assimilate":"BSD",
		"type":"BSD-3-clauses"
		}

def create(target):
	myModule = module.Module(__file__, 'enettest', 'BINARY')
	
	myModule.add_module_depend(['enet'])
	
	myModule.add_src_file([
		'test/debug.cpp',
		'test/main.cpp'
		])
	
	myModule.add_export_path(tools.get_current_path(__file__))
	
	# add the currrent module at the 
	return myModule







