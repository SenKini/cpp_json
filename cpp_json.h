#include <string>
#include <vector>
#include <variant>
#include <memory>

#define JsonAbstractPtr std::shared_ptr<JsonAbstract>
#define JsonNumberPtr std::shared_ptr<JsonNumber>
#define JsonStringPtr std::shared_ptr<JsonString>
#define JsonBooleanPtr std::shared_ptr<JsonBoolean>
#define JsonObjectPtr std::shared_ptr<JsonObject>
#define JsonArrayPtr std::shared_ptr<JsonArray>

namespace DevKit {
enum JsonType {
	OBJECT,
	ARRAY,
	STRING,
	NUMBER,
	BOOLEAN,
	NUL
};

// 抽象类
class JsonAbstract {
protected:
	std::string _key;
	JsonType _type;	 // 类型

public:
	JsonAbstract(std::string k, JsonType t);
	virtual ~JsonAbstract() = default;
	std::string getKey();				 // 获取 key
	JsonType getType();					 // 获取类型
	virtual std::string toString() = 0;	 // 转换为字符串形式（未格式化）
	virtual std::string getValue() = 0;	 // 获取字符串形式的 value
};

// 数字
class JsonNumber : public JsonAbstract {
private:
	std::variant<long, double> _value;

public:
	JsonNumber(std::string k, long v);
	JsonNumber(std::string k, double v);
	JsonNumber(std::string k, std::variant<long, double> v);
	virtual std::string toString();
	virtual std::string getValue();
};

// 字符串
class JsonString : public JsonAbstract {
private:
	std::string _value;

public:
	JsonString(std::string k, std::string v);
	virtual std::string toString();
	virtual std::string getValue();
};

// 布尔值
class JsonBoolean : public JsonAbstract {
private:
	bool _value;

public:
	JsonBoolean(std::string k, bool v);
	virtual std::string toString();
	virtual std::string getValue();
};

// 对象
class JsonObject : public JsonAbstract {
private:
	using ObjPair = std::pair<std::string, JsonAbstractPtr>;
	std::vector<ObjPair> _items;  // 存储子 json

public:
	JsonObject();
	JsonAbstractPtr operator[](std::string key);
	void add(JsonAbstractPtr item);
	void add(std::string key, JsonAbstractPtr item);
	virtual std::string toString();
	virtual std::string getValue();
};

// 数组
class JsonArray : public JsonAbstract {
private:
	std::vector<JsonAbstractPtr> _items;  // 存储子 json

public:
	JsonArray();
	JsonAbstractPtr operator[](int pos);
	void add(JsonAbstractPtr item);
	virtual std::string toString();
	virtual std::string getValue();
};

// 解析器
class JsonParse {
private:
	const std::string _str;
	int _index;

	bool isCross();								 //_index 是否越界
	void skip();								 // 跳过无关字符
	std::string extractKey();					 // 提取 key
	std::variant<long, double> extractNumber();	 // 提取 number
	std::string extractString();				 // 提取 string
	bool extractBoolean();						 // 提取 boolean
	JsonAbstractPtr parseObject();				 // 提取对象
	JsonAbstractPtr parseArray();				 // 提取数组

public:
	JsonParse(std::string s);
	JsonObjectPtr parse();  // 解析字符串
};
}  // namespace DevKit