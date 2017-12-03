#pragma once
#ifdef _WIN32
#include <crtdbg.h>
#define CODEX_ASSERT(expression) _ASSERT(expression)
#else
#include "Codex/Log.h"
#define CODEX_ASSERT(expression) { if (!(expression)) codex::log::error("CODEX_ASSERT failed. Expression: " #expression); }
#endif