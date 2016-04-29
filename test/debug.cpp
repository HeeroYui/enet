/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test/debug.h>

int32_t appl::getLogId() {
	static int32_t g_val = elog::registerInstance("enettest");
	return g_val;
}
