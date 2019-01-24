#pragma once

// #define  USELIB_CURL
 #define  USELIB_WININET
// 

#if defined USELIB_CURL
#define  CURL_STATICLIB
#define  HTTP_ONLY
#pragma comment(lib, "WS2_32")
#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Normaliz.lib")
#pragma comment(lib, "libcurl.lib")
#elif defined USELIB_WININET
#pragma comment(lib, "WinINet.lib")
#else 
#endif 




