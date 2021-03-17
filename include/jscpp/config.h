#ifndef __JSCPP_CONFIG_H__
#define __JSCPP_CONFIG_H__

#ifdef __cplusplus
  #define EXTERN_C_START extern "C" {
  #define EXTERN_C_END }
#else
  #define EXTERN_C_START
  #define EXTERN_C_END
#endif

#ifdef _WIN32
  #ifdef JSCPP_BUILD_DLL
    #define JSCPP_API __declspec(dllexport)
  #else
    #ifdef JSCPP_IMPORT_DLL
      #define JSCPP_API __declspec(dllimport)
    #else
      #define JSCPP_API
    #endif
  #endif
#else
  #ifdef JSCPP_BUILD_DLL
    #define JSCPP_API __attribute__((visibility("default")))
  #else
    #define JSCPP_API
  #endif
#endif

#endif
