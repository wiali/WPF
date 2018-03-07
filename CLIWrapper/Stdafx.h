// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifdef BIRNABY_EXPORTS
#define BIRNABY_API __declspec(dllexport)
#else
#define BIRNABY_API __declspec(dllimport)
#endif

