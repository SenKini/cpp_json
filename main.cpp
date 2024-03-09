#include "cpp_json.h"
#include <iostream>

using namespace DevKit;

int main() {
	JsonObjectPtr jop;
	JsonParse jp(R"(
            {
                "na,me"   :    "John",
                "a:ge"   : 30,    
                "isStude nt": false,
                "grades"  : [90,   85, 95],
                "address": {
               "city": "New York",
                    "country": "USA"
            }
        })");

	jop = jp.parse();
	std::cout << jop->toString() << std::endl;
	std::cout << (*jop)["na,me"]->getValue() << std::endl;
	std::cout << (*jop)["grades"]->getValue() << std::endl;
	std::cout << (*jop)["address"]->toString() << std::endl;

    return 0;
}