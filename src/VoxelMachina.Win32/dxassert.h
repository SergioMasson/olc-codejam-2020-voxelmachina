#pragma once

//Utilitie functions to help debuging HRESULT.

inline void Print(const char* msg) { OutputDebugStringA(msg); }
inline void Print(const wchar_t* msg) { OutputDebugStringW(msg); }

inline void Printf(const char* format, ...)
{
	char buffer[256];
	va_list ap;
	va_start(ap, format);
	vsprintf_s(buffer, 256, format, ap);
	Print(buffer);
}

inline void Printf(const wchar_t* format, ...)
{
	wchar_t buffer[256];
	va_list ap;
	va_start(ap, format);
	vswprintf(buffer, 256, format, ap);
	Print(buffer);
}

#ifndef RELEASE
inline void PrintSubMessage(const char* format, ...)
{
	Print("--> ");
	char buffer[256];
	va_list ap;
	va_start(ap, format);
	vsprintf_s(buffer, 256, format, ap);
	Print(buffer);
	Print("\n");
}
inline void PrintSubMessage(const wchar_t* format, ...)
{
	Print("--> ");
	wchar_t buffer[256];
	va_list ap;
	va_start(ap, format);
	vswprintf(buffer, 256, format, ap);
	Print(buffer);
	Print("\n");
}

inline void PrintSubMessage(void)
{
}
#endif

#define STRINGIFY(x) #x
#define STRINGIFY_BUILTIN(x) STRINGIFY(x)
#define ASSERT( isFalse, ... ) \
        if (!(bool)(isFalse)) { \
            Print("\nAssertion failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
            PrintSubMessage("\'" #isFalse "\' is false"); \
            PrintSubMessage(__VA_ARGS__); \
            Print("\n"); \
            __debugbreak(); \
        }

#define ASSERT_SUCCEEDED( hr, ... ) \
        if (FAILED(hr)) { \
            Print("\nHRESULT failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
            PrintSubMessage("hr = 0x%08X", hr); \
            PrintSubMessage(__VA_ARGS__); \
			Print("\n"); \
			__debugbreak(); \
		}