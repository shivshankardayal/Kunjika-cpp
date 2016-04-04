#ifndef BASE64_h
#define BASE64_h

#include <string>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);

#endif // base64.h