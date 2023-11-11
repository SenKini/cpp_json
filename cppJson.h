#include <string>
#include <unordered_map>
#include <list>
#include <any>
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
	CppJsonItem(CppJsonType type, std::string key, std::any value);	 // 创建基础类型的 item
	CppJsonItem(CppJsonType type, std::string key);					 // 创建对象或数组类型的 item
	CppJsonItem(CppJsonType type);									 // 创建最外层
	std::any getValue(std::string key);								 // 通过 value 参数传回值
	std::string getKey();											 // 获取关键字
	CppJsonType getType();											 // 获取类型
	bool add(CppJsonItem *item);									 // 添加新的子 item
	bool erase(std::string key);									 // 移除子 item
	std::string print();											 // 返回 json 格式字符串
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