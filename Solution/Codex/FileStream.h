#pragma once
#include <string>
#include <fstream>
#include <vector>
namespace codex
{
	/**Returns true if directory exists at given path*/
	bool directoryExists(const std::string& path);

	/**Returns true if file exists at given path*/
	bool fileExists(const std::string& path);

	/**Returns true if file exists and was removed succesfully*/
	bool removeFile(const std::string& path);

	/**Returns false if failed to create directory at given path*/
	bool createDirectory(const std::string& path);

	/**If directory doesn't exist, creates one. If creation fails, returns false*/
	bool verifyDirectory(const std::string& path);

	/**Returns a vector of strings, each string representing a file found in the given directory.
	Directory path is not included in the file string. If file type is specified, the file type will not be included in the file string.
	If file type is specified, only files of that type are returned.*/
	std::vector<std::string> listFilesInDirectory(const std::string& directoryPath, std::string fileType = "");

	/**Returns a vector of strings, each string representing a sub directory found in the given directory.
	Directory path is not included in the sub directory string.*/
	std::vector<std::string> listSubDirectoriesInDirectory(const std::string& directoryPath);
}