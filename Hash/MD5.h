#ifndef ROOTIVE_MD5_H
#define ROOTIVE_MD5_H

#include <string>
#include <cstring>

namespace Rootive
{
class MD5
{
	struct Context
	{
		unsigned long int state[4];
		unsigned long int count[2];
		unsigned char buffer[64];
	};
	Context context_;
	static const unsigned char padding_[64];
	void _transform(unsigned long int state[4], const unsigned char block[64]);
	void _encode(unsigned char* output, const unsigned long int* input, unsigned int len);
	void _decode(unsigned long int* output, const unsigned char* input, unsigned int len);
	void _memcpy(unsigned char* output, const unsigned char* input, unsigned int len);
	void _memset(unsigned char* output, int value, unsigned int len);
	void _init(Context* context);
	void _update(Context* context, const unsigned char* input, unsigned int inputLen);
	void _final(unsigned char digest[16], Context* context);
public:
	std::string get(const char* data, size_t length);
    inline std::string get(const char* data) { return get(data, strlen(data)); }
};
}

#endif