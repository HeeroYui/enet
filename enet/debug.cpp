/**
 * @author Edouard DUPIN
 * 
 * @copyright 2014, Edouard DUPIN, all right reserved
 * 
 * @license BSD 3 clauses (see license file)
 */

#include <enet/debug.h>

int32_t enet::getLogId(void) {
	static int32_t g_val = etk::log::registerInstance("enet");
	return g_val;
}
