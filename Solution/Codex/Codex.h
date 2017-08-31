#pragma once
#ifdef GHOST_CODEX
#ifdef SHELL_CODEX
#error Both GHOST_CODEX and SHELL_CODEX have been defined!
#endif
#endif
#ifndef SHELL_CODEX
#ifndef GHOST_CODEX
#error Neither GHOST_CODEX or SHELL_CODEX has been defined!
#endif
#endif
#include <string>

namespace codex
{
	/*
	Codexes are implemented in two types.
	Shell codex
	Used by software used in robots.
	The shell codex implementation favours portability and efficiency.
	Ghost codex
	Ghost codex is used by the remote server software.
	Server codex implementation favours local computation over remote computation.
	*/
	enum class CodexType : uint8_t
	{
		invalid,
		ghost,
		shell,
	};
	extern const CodexType codexType;

	int initialize(const int argc, const char** argv);
	void uninitialize();

	extern std::string workingDirectory;
}