/** @file
 * @author Edouard DUPIN
 * @copyright 2018, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <etk/types.hpp>
#include <test-debug/debug.hpp>
#include <etest/etest.hpp>
#include <enet/pourcentEncoding.hpp>

TEST(pourcentSerializer, stringEncode) {
	EXPECT_EQ(enet::pourcentEncode("hello"), "hello");
	EXPECT_EQ(enet::pourcentEncode("hello@plop plouf-_~"), "hello%40plop%20plouf-_~");
}

TEST(pourcentSerializer, stringDecode) {
	EXPECT_EQ(enet::pourcentDecode("hello"), "hello");
	EXPECT_EQ(enet::pourcentDecode("hello%40plop%20plouf-_~"), "hello@plop plouf-_~");
}

TEST(pourcentSerializer, mapEncode) {
	etk::Map<etk::String, etk::String> data;
	data.set("hello", "value1");
	data.set("simpleKey", "");
	data.set("special Key", "coucou");
	data.set("email", "you@example.com");
	EXPECT_EQ(enet::pourcentUriEncode(data), "email=you%40example.com&hello=value1&simpleKey&special%20Key=coucou");
}

TEST(pourcentSerializer, mapDecode) {
	auto data = enet::pourcentUriDecode("hello=value1&simpleKey&special%20Key=coucou&email=you%40example.com");
	EXPECT_EQ(data.size(), 4);
	EXPECT_EQ(data["hello"], "value1");
	EXPECT_EQ(data["simpleKey"], "");
	EXPECT_EQ(data["special Key"], "coucou");
	EXPECT_EQ(data["email"], "you@example.com");
}
