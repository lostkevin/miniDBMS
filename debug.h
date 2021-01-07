#pragma once
#include <iostream>
#ifdef _DEBUG_
#define LOG(mod, content) std::cout << (mod) << ":" << (content) << std::endl
#else
#define LOG(mod, content)
#endif
