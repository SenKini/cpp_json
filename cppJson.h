#include <string>
#include <unordered_map>
#include <list>
#include <variant>

enum CppJsonType {
	RAW,
	INVALID,
	BOOL,
	NUL,
	INT,
	DOUBLE,
	STRING,
	ARRAY,
	OBJECT
};	// json 类型

using CPPJSON_VALUE = std::variant<bool, int, double, std::string>;

// 基本键值对
class CppJsonItem {
private:
	CppJsonType _type;				   // 类型
	std::string _key;				   // 关键字
	CPPJSON_VALUE _value;			   // 值
	std::list<CppJsonItem *> _childs;  // 子对象

public:
	template <class ValueType>
	CppJsonItem(CppJsonType type, std::string key, ValueType value);  // 创建基础类型的 item
	CppJsonItem(CppJsonType type, std::string key);					  // 创建对象或数组类型的 item
	CppJsonItem(CppJsonType type);									  // 创建最外层
	std::string getKey();											  // 获取关键字
	CppJsonType getType();											  // 获取类型
	bool add(CppJsonItem *item);									  // 添加新的子 item
	bool erase(std::string key);									  // 移除子 item
	std::string print();											  // 返回 json 格式字符串
	CppJsonItem *getItem(std::string key);							  // 获取 CppJsonItem 对象
	auto getValue(std::string key) {
		if ((_type != OBJECT) && (_type != ARRAY) && (_type != NUL))  // 普通 item
			return _value;
		else if (_type != NUL)	// 复合 item
			for (auto child : _childs)
				if (child->_key == key)
					return child->_value;

		return CPPJSON_VALUE();
	}  // 通过 key 获取当前对象中的值
};

// 最外层 json，使用单件模式
class CppJsonSingleton {
private:
	static CppJsonSingleton *_cppJsonSingleton;
	CppJsonItem *_child;						  // 子对象
	CppJsonItem *stringToJson(std::string json);  // 将 json 字符串转为 CppJsonItem
	std::string clearJson(std::string json);	  // 清理 json 字符串中多余的符号
	bool isValidJson(std::string json);			  // TODO: 检查是否为合法的 json 字符串

public:
	static CppJsonSingleton *getCppJsonSingleton();	 // 只能允许一个实例被创建
	void import(std::string json);					 // 导入已经存在的 json
	std::string print();
	void add(CppJsonItem *item);
	bool erase(std::string key);
};

// 按照 json 格式返回字符串，使用策略模式
class CppJsonStrategy {
public:
	std::string printSimple(std::string key, CPPJSON_VALUE value);									 // 对简单结构的打印
	std::string printCompound(std::string key, CppJsonType type, std::list<CppJsonItem *> *childs);	 // 对复合结构的打印
	std::string printRaw(std::list<CppJsonItem *> *childs);											 // 对最外层的打印
};

template <class ValueType>
CppJsonItem::CppJsonItem(CppJsonType type, std::string key, ValueType value)
	: _key(key), _type(type) {
	if (type != BOOL && type != INT && type != DOUBLE && type != STRING)
		_type = INVALID;
	else
		_value = value;
}