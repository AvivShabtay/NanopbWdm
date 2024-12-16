#pragma once

/**
	https://stackoverflow.com/a/38237385
*/
constexpr const char* getFilename(const char* path)
{
	const char* file = path;
	while (*path)
	{
		if (*path++ == '\\')
		{
			file = path;
		}
	}
	return file;
}

#define PRINT(fmt, ...)			\
	DbgPrint(fmt, __VA_ARGS__);	\

#define PRINT_FILENAME() do {								\
	constexpr const char*_filename = getFilename(__FILE__); \
	PRINT("%s", _filename);									\
} while(0);

#define PRINT_LINE_AND_FUNCTION()			\
	PRINT("(%s : %d)", __func__, __LINE__);	\

#define DEBUG_LOG_WITHOUT_NEW_LINE(format, ...) do {	\
	PRINT_FILENAME();				\
	PRINT(" ");						\
	PRINT_LINE_AND_FUNCTION();		\
	PRINT(": ");					\
	PRINT(format, __VA_ARGS__);		\
} while (0);

#define DEBUG_LOG(format, ...) do {					\
	DEBUG_LOG_WITHOUT_NEW_LINE(format, __VA_ARGS__);\
	PRINT("\n");									\
} while (0)
