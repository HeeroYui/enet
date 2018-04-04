/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <enet/pourcentEncoding.hpp>
#include <enet/debug.hpp>
#include <etk/types.hpp>
#include <etk/String.hpp>

const etk::String hexData = "0123456789ABCDEF";

etk::String enet::pourcentEncode(const etk::String& _data) {
	etk::String out;
	for (auto &it : _data) {
		if (    (it >= 'a' && it <= 'z')
		     || (it >= 'A' && it <= 'Z')
		     || (it >= '0' && it <= '9')
		     || it == '-'
		     || it == '_'
		     || it == '.'
		     || it == '~') {
			out += it;
		} else {
			out += "%";
			out += hexData[(uint32_t(it)>>4)&0x0F];
			out += hexData[uint32_t(it)&0x0F];
		}
	}
	return out;
}

static int32_t convertStringHexToInt(const char _value) {
	if (_value >= 'a' && _value <= 'z') {
		return int32_t(_value) - int32_t('a') + 10;
	}
	if (_value >= 'A' && _value <= 'Z') {
		return int32_t(_value) - int32_t('A') + 10;
	}
	if (_value >= '0' && _value <= '9') {
		return int32_t(_value) - int32_t('0');
	}
	ENET_ERROR("Not a hexadecimal Value: '" << _value << "'");
	return 0;
}

etk::String enet::pourcentDecode(const etk::String& _data) {
	etk::String out;
	for (size_t iii=0; iii<_data.size(); ++iii) {
		auto it = _data[iii];
		if (it == '%') {
			if (iii+2 < _data.size()) {
				auto val1 = convertStringHexToInt(_data[iii+1])<<4;
				val1 += convertStringHexToInt(_data[iii+2]);
				out += char(val1);
				iii += 2;
			} else {
				ENET_ERROR("can not convert pourcent ==> input size error: '" << _data << "'");
				return out;
			}
		} else {
			out += it;
		}
	}
	return out;
}

etk::String enet::pourcentUriEncode(const etk::Map<etk::String, etk::String>& _data) {
	etk::String out;
	for (auto &it: _data) {
		if (out.empty() == false) {
			out += "&";
		}
		out += enet::pourcentEncode(it.first);
		if (it.second.empty() == false) {
			out += "=";
			out += enet::pourcentEncode(it.second);
		}
	}
	return out;
}

etk::Map<etk::String, etk::String> enet::pourcentUriDecode(const etk::String& _data) {
	etk::Map<etk::String, etk::String> out;
	auto listElements = etk::split(_data, '&');
	for (auto &it : listElements) {
		if (it.size() == 0) {
			continue;
		}
		auto offset = it.find('=');
		if (offset == etk::String::npos) {
			out.set(enet::pourcentDecode(it), "");
			continue;
		}
		out.set(enet::pourcentDecode(it.extract(0, offset)),
		        enet::pourcentDecode(it.extract(offset+1, etk::String::npos)));
	}
	return out;
}

