/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once


#include <etk/String.hpp>
#include <etk/Map.hpp>


namespace enet {
	/**
	 * @brief Encode a string with transform the unvalid char in %XX.
	 * @param[in] _data Raw string.
	 * @return Encoded string.
	 */
	etk::String pourcentEncode(const etk::String& _data);
	
	/**
	 * @brief Decode a string with %XX with normal chars.
	 * @param[in] _data Encoded string.
	 * @return Raw string.
	 */
	etk::String pourcentDecode(const etk::String& _data);
	
	/**
	 * @brief Encode a map of string with %XX and "&" between element and = between key and value.
	 * @param[in] _data Imput map to encode.
	 * @return Encoded string.
	 */
	etk::String pourcentUriEncode(const etk::Map<etk::String, etk::String>& _data);
	
	/**
	 * @brief Encode a map of string with %XX and "&" between element and = between key and value.
	 * @param[in] _data Encoded string like AAA=333&RRR=333&RRR
	 * @return Te decoded map of values.
	 */
	etk::Map<etk::String, etk::String> pourcentUriDecode(const etk::String& _data);
	
}

