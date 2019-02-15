#pragma once

// #define  USELIB_CURL
 #define  USELIB_WININET
// 

#if defined USELIB_CURL
#define  CURL_STATICLIB
#elif defined USELIB_WININET
#else 
#endif 




