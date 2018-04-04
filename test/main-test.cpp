/** @file
 * @author Edouard DUPIN
 * @copyright 2018, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <etk/types.hpp>
#include <test-debug/debug.hpp>


#include <etest/etest.hpp>

int main(int argc, const char *argv[]) {
	// init test engine:
	etest::init(argc, argv);
	TEST_INFO("TEST ETK");
	return RUN_ALL_TESTS();
}
