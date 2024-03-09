#include "cpp_json.h"

using namespace DevKit;

// ------------------------------ JsonAbstract ------------------------------ //
JsonAbstract::JsonAbstract(std::string k, JsonType t)
	: _key(k), _type(t) {}

std::string DevKit::JsonAbstract::getKey() {
	return _key;
}

JsonType JsonAbstract::getType() {
	return _type;
}

// ------------------------------ JsonNumber ------------------------------ //
JsonNumber::JsonNumber(std::string k, long v)
	: _value(v), JsonAbstract(k, NUMBER) {}

JsonNumber::JsonNumber(std::string k, double v)
	: _value(v), JsonAbstract(k, NUMBER) {}

DevKit::JsonNumber::JsonNumber(std::string k, std::variant<long, double> v)
	: _value(v), JsonAbstract(k, NUMBER) {}

std::string JsonNumber::toString() {
	if (_value.index() == 0) {
		long longValue = std::get<long>(_value);
		return "\"" + _key + "\":" + std::to_string(longValue);
	}
	else {
		double doubleValue = std::get<double>(_value);
		return "\"" + _key + "\":" + std::to_string(doubleValue);
	}
}

std::string DevKit::JsonNumber::getValue() {
	if (_value.index() == 0)
		return std::to_string(std::get<long>(_value));
	else
		return std::to_string(std::get<double>(_value));
}

// ------------------------------ JsonString ------------------------------ //
JsonString::JsonString(std::string k, std::string v)
	: _value(v), JsonAbstract(k, STRING) {}

std::string JsonString::toString() {
	return "\"" + _key + "\":" + "\"" + _value + "\"";
}

std::string JsonString::getValue() {
	return _value;
}

// ------------------------------ JsonBoolean ------------------------------ //
JsonBoolean::JsonBoolean(std::string k, bool v)
	: _value(v), JsonAbstract(k, BOOLEAN) {}

std::string JsonBoolean::toString() {
	return "\"" + _key + "\":" + (_value ? "true" : "false");
}

std::string JsonBoolean::getValue() {
	return (_value ? "true" : "false");
}

// ------------------------------ JsonObject ------------------------------ //
JsonObject::JsonObject()
	: JsonAbstract("", OBJECT) {
	_items.reserve(10);
}

JsonAbstractPtr JsonObject::operator[](std::string key) {
	// 使用遍历的方式查找
	for (auto pair : _items)
		if (pair.first == key)
			return pair.second;

	return nullptr;
}

void JsonObject::add(JsonAbstractPtr item) {
	switch (item->getType()) {
		case NUMBER:
			_items.emplace_back(item->getKey(), item);
			break;

		case STRING:
			_items.emplace_back(item->getKey(), item);
			break;

		case BOOLEAN:
			_items.emplace_back(item->getKey(), item);
			break;
	}
}

void JsonObject::add(std::string key, JsonAbstractPtr item) {
	switch (item->getType()) {
		case OBJECT:
		case ARRAY:
			_items.emplace_back(key, item);
			break;
	}
	_items.emplace_back(key, item);
}

std::string JsonObject::toString() {
	std::string res = "{";

	// 使用递归拼出整个字符串
	for (auto pair : _items)
		switch (pair.second->getType()) {
			case NUMBER:
			case STRING:
			case BOOLEAN:
				res.append(pair.second->toString() + ",");
				break;

			case OBJECT:
			case ARRAY:
				res.append(pair.first + ":" + pair.second->toString() + ",");
				break;
		}

	res.pop_back();	 // 弹出多余的逗号
	res.push_back('}');

	return res;
}

std::string DevKit::JsonObject::getValue() {
	std::string res = toString();

	// 取出最外层的括号
	res.erase(res.begin());
	res.erase(res.end() - 1);

	return res;
}

// ------------------------------ JsonArray ------------------------------ //
JsonArray::JsonArray()
	: JsonAbstract("", ARRAY) {}

JsonAbstractPtr JsonArray::operator[](int pos) {
	return _items[pos];
}

void JsonArray::add(JsonAbstractPtr item) {
	_items.emplace_back(item);
}

std::string JsonArray::toString() {
	std::string res = "[";

	for (auto item : _items)
		res.append(item->toString().substr(3) + ",");

	res.pop_back();	 // 弹出多余的逗号
	res.push_back(']');

	return res;
}

std::string DevKit::JsonArray::getValue() {
	std::string res = toString();

	// 取出最外层的括号
	res.erase(res.begin());
	res.erase(res.end() - 1);

	return res;
}

// ------------------------------ JsonParse ------------------------------ //
bool DevKit::JsonParse::isCross() {
	int size = _str.size();
	return !(_index < _str.size());
}

void JsonParse::skip() {
	while (!isCross())
		switch (_str[_index]) {
			case '\n':
			case '\t':
			case ' ':
			case ',':
			case ':':
				_index++;
				break;
			default:
				return;
		}
}

std::string JsonParse::extractKey() {
	std::string key;

	if (!isCross()) {
		_index++;
		skip();
		for (; _str[_index] != '\"'; _index++)
			key.push_back(_str[_index]);
		_index++;
	}

	return key;
}

std::variant<long, double> DevKit::JsonParse::extractNumber() {
	std::variant<long, double> res;

	std::string value;
	for (; !isCross() && std::isdigit(_str[_index]); _index++)
		value.push_back(_str[_index]);

	if (value.find('.') == std::string::npos)
		res = std::stol(value);
	else
		res = std::stod(value);

	return res;
}

std::string DevKit::JsonParse::extractString() {
	std::string res;

	// 一直提取直到遇到引号
	for (; !isCross() && (_str[_index] != '\"'); _index++)
		res.push_back(_str[_index]);
	_index++;  // 去掉末尾引号

	return res;
}

bool DevKit::JsonParse::extractBoolean() {
	_index += (_str[_index] == 't') ? 4 : 5;  // 根据开头字母直接跳过

	return (_str[_index] == 't');
}

JsonParse::JsonParse(std::string s)
	: _str(s), _index(1) {}

JsonAbstractPtr DevKit::JsonParse::parseObject() {
	JsonObjectPtr res(new JsonObject);

	while (!isCross() && (_index < _str.size()) && (_str[_index] != '}')) {
		skip();
		_index++;							// 去掉起始引号
		std::string key = extractString();	// 得到子 json 的 key
		skip();

		if (isCross())
			break;

		// 针对不同的类型使用不同提取方法
		switch (_str[_index]) {
			case '{':
				_index++;  // 去掉起始括号
				res->add(key, parseObject());
				break;

			case '[':
				_index++;  // 去掉起始括号
				res->add(key, parseArray());
				break;

			case '\"': {
				_index++;  // 去掉起始引号
				JsonStringPtr jsp(new JsonString(key, extractString()));
				res->add(std::static_pointer_cast<JsonAbstract>(jsp));
				break;
			}

			case 't':
			case 'f': {
				JsonBooleanPtr jbp(new JsonBoolean(key, extractBoolean()));
				res->add(std::static_pointer_cast<JsonAbstract>(jbp));
				break;
			}

			default: {
				JsonNumberPtr jnp(new JsonNumber(key, extractNumber()));
				res->add(std::static_pointer_cast<JsonAbstract>(jnp));
				break;
			}
		}
	}
	_index++;  // 去掉末尾引号

	return std::static_pointer_cast<JsonAbstract>(res);
}

JsonAbstractPtr DevKit::JsonParse::parseArray() {
	JsonArrayPtr res(new JsonArray);

	while (!isCross() && (_index < _str.size()) && (_str[_index] != ']')) {
		skip();

		if (isCross())
			break;

		// 针对不同的类型使用不同提取方法
		switch (_str[_index]) {
			case '{':
				_index++;
				res->add(parseObject());
				break;

			case '[':
				_index++;
				res->add(parseArray());
				break;

			case '\"': {
				JsonStringPtr jsp(new JsonString("", extractString()));
				res->add(std::static_pointer_cast<JsonAbstract>(jsp));
				break;
			}

			case 't':
			case 'f': {
				JsonBooleanPtr jbp(new JsonBoolean("", extractBoolean()));
				res->add(std::static_pointer_cast<JsonAbstract>(jbp));
				break;
			}

			default: {
				JsonNumberPtr jnp(new JsonNumber("", extractNumber()));
				res->add(std::static_pointer_cast<JsonAbstract>(jnp));
				break;
			}
		}
	}
	_index++;  // 去掉末尾引号

	return std::static_pointer_cast<JsonAbstract>(res);
}

JsonObjectPtr DevKit::JsonParse::parse() {
	skip();
	_index++;  // 去掉起始括号
	return std::static_pointer_cast<JsonObject>(parseObject());
}
