#ifndef __getTypeString_H__
#define __getTypeString_H__
#include <stdint.h>
#include <cxxabi.h>
#include <string>


template <typename T>
std::string getTypeName()
{
    auto typeCode = typeid(T).name();
    int status;
    auto _typeName = abi::__cxa_demangle(typeCode, NULL, NULL, &status);
    if (status != 0)
    {
        // fail
        return std::string("NONE");
        
    }
    auto typeNameStr = std::string(_typeName);
    std::free(_typeName);
    return typeNameStr;
}

#endif
