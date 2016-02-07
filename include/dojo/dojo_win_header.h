#pragma once

#ifndef WIN32
#error only include this file on Windows!
#endif

#define NOMINMAX 1  //Y U NO LEAVE STL ALONE
#define WIN32_LEAN_AND_MEAN 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <windows.h>
#include <ShellAPI.h>
#include <ShlObj.h>
