#include "cppJson.h"
#include <algorithm>
#include <stack>

CppJsonItem::CppJsonItem(CppJsonType type, std::string key, std::any value)
	: _key(key), _type(type) {
	switch (type) {
		case BOOL:
			_value = std::any_cast<bool>(value);
			break;
		case INT:
			_value = std::any_cast<int>(value);
			break;
		case DOUBLE:
			_value = std::any_cast<double>(value);
			break;
		case STRING:
			_value = std::any_cast<std::string>(value);
			break;
		default:
			_type = INVALID;  // 和另一个重载函数统一
	}
}

CppJsonItem::CppJsonItem(CppJsonType type, std::string key) {
	switch (type) {
		case ARRAY:
		case OBJECT:
		case NUL:
			_type = type;
			_key = key;
			break;
		default:
			_type = INVALID;
	}
}

CppJsonItem::CppJsonItem(CppJsonType type) {
	_type = (type == RAW) ? RAW : INVALID;
}

std::any CppJsonItem::getValue(std::string key) {
	if ((_type != OBJECT) && (_type != ARRAY) && (_type != NUL))  // 普通 item
		return _value;
	else if (_type != NUL)	// 复合 item
		for (auto child : _childs)
			if (child->_key == key)
				return child->_value;

	return std::any();
}

std::string CppJsonItem::getKey() {
	if (_type != INVALID)
		return _key;
	else
		return "error";
}

CppJsonType CppJsonItem::getType() {
	return _type;
}

bool CppJsonItem::add(CppJsonItem *item) {
	if (((_type != OBJECT) && (_type != ARRAY)) && (_type != RAW) && (_type == NUL))
		return false;
	else {
		_childs.push_back(item);
		return true;
	}
}

bool CppJsonItem::erase(std::string key) {
	if (_childs.empty())  // NUL 类型的 _childs 也为空，故不另做判断
		return false;
	else
		for (auto child = _childs.begin(); child != _childs.end(); child++)
			if ((*child)->_key == key) {
				delete *child;
				_childs.erase(child);
				return true;
			}

	return false;
}

std::string CppJsonItem::print() {
	CppJsonStrategy strategy;

	if (_type == BOOL || _type == INT || _type == DOUBLE || _type == STRING)
		return strategy.printSimple(_key, _value);
	else if (_type == OBJECT || _type == ARRAY)
		return strategy.printCompound(_key, _type, &_childs);
	else if (_type == RAW)
		return strategy.printRaw(&_childs);
	else
		return _key + ":null";	// 对于 NUL 类型单独处理
}

CppJsonSingleton *CppJsonSingleton::_cppJsonSingleton = nullptr;

CppJsonItem *CppJsonSingleton::stringToJson(std::string json) {
	std::stack<CppJsonItem *> itemStk;	   // 存储 item 的栈，模拟递归
	auto currItem = new CppJsonItem(RAW);  // 当前使用的 item
	std::string key;					   // 关键字
	std::string value;					   // 值，暂时为 string，后面会进行转换

	// 遍历整个字符串
	for (int i = 0; i < json.size(); i++) {
		// 第一个引号一定是关键字的
		if (json[i] == '\"')
			for (i++; json[i] != '\"'; i++)
				key.push_back(json[i]);

		// 冒号代表键值分隔
		if (json[i] == ':') {
			if (json[i + 1] == '{') {  // 遇到子项目
				itemStk.push(currItem);
				currItem = new CppJsonItem(OBJECT, key);
				key.clear();
				i++;
			}
			else if (json[i + 1] == '[') {	// 遇到子数组
				itemStk.push(currItem);
				currItem = new CppJsonItem(ARRAY, key);
				key.clear();
				i++;
			}
			else {	// 普通的键值对
				CppJsonItem *tmpItem;

				// 提取值部分
				for (i++; (json[i] != ',') && (json[i] != '}') && (json[i] != ']'); i++)
					value.push_back(json[i]);

				if (value[0] == '\"') {	 // 是否为字符串
					value.erase(value.begin());
					value.erase(value.end() - 1);

					tmpItem = new CppJsonItem(STRING, key, value);
				}
				else if (std::find(value.begin(), value.end(), '.') == value.end()) {  // 是否为整数
					tmpItem = new CppJsonItem(INT, key, std::stoi(value));
				}
				else {
					tmpItem = new CppJsonItem(DOUBLE, key, std::stod(value));
				}

				currItem->add(tmpItem);	 // 加入新的键值对
				key.clear();
				value.clear();
			}
		}
		else if (json[i] == '}' || json[i] == ']') {  // 遇到子项目或子数组结束，回溯
			itemStk.top()->add(currItem);
			currItem = itemStk.top();
			itemStk.pop();
		}
	}

	return currItem;  // 此时返回的是最初的 item
}

std::string CppJsonSingleton::clearJson(std::string json) {
	std::string res;

	for (char ch : json)
		if ((ch != '\n') && (ch != '\t'))
			res.push_back(ch);

	return res;
}

CppJsonSingleton *CppJsonSingleton::getCppJsonSingleton() {
	if (_cppJsonSingleton == nullptr) {
		_cppJsonSingleton = new CppJsonSingleton();
		_cppJsonSingleton->_child = new CppJsonItem(RAW);
	}

	return _cppJsonSingleton;
}

void CppJsonSingleton::import(std::string json) {
	std::string cleanJson = clearJson(json);

	_child = stringToJson(cleanJson);
}

std::string CppJsonSingleton::print() {
	return _child->print();
}

void CppJsonSingleton::add(CppJsonItem *item) {
	_child->add(item);
}

bool CppJsonSingleton::erase(std::string key) {
	return _child->erase(key);
}

std::string CppJsonStrategy::printSimple(std::string key, CPPJSON_VALUE value) {
	std::string res;	   // 最后结果
	std::string valueStr;  // 值的字符串形式

	// 根据值的具体类型决定处理方式
	switch (value.index()) {
		case 0:
			valueStr = std::get<bool>(value) ? "True" : "False";
			break;
		case 1:
			valueStr = std::to_string(std::get<int>(value));
			break;
		case 2:
			valueStr = std::to_string(std::get<double>(value));
			break;
		case 3:
			valueStr = "\"" + std::get<std::string>(value) + "\"";
			break;
	}

	key = "\"" + key + "\"";
	res = key + ":" + valueStr;

	return res;
}

std::string CppJsonStrategy::printCompound(std::string key, CppJsonType type, std::list<CppJsonItem *> *childs) {
	std::string res;	   // 最后结果
	std::string childStr;  // 子项目转换为字符串后的形式

	// 递归打印
	for (auto child : *childs)
		childStr.append(child->print() + ",");

	childStr.erase(childStr.end() - 1);	 // 删除多余的逗号

	// 添加括号
	key = "\"" + key + "\"";
	if (type == OBJECT)
		res = key + ":{" + childStr + "}";
	else
		res = key + ":[" + childStr + "]";

	return res;
}

std::string CppJsonStrategy::printRaw(std::list<CppJsonItem *> *childs) {
	std::string res;	   // 最后结果
	std::string childStr;  // 子项目转换为字符串后的形式

	// 递归打印
	for (auto child : *childs)
		childStr.append(child->print() + ",");

	childStr.erase(childStr.end() - 1);	 // 删除多余的逗号

	// 添加括号
	res = "{" + childStr + "}";

	return res;
}
