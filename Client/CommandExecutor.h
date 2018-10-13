#pragma once

#include <string>
#include <boost/asio.hpp>

class CommandExecutor
{
	static const std::string serverAddress;
	static const std::string serverPort;

	boost::asio::io_context context;
	boost::asio::ip::tcp::socket socket;
	boost::asio::ip::tcp::resolver resolver;

	/** Check if the given file is "." or "..". */
	void connectToServerAndCheck(std::string path, bool isDirectory);
	bool checkIfDefaultFile(const char *name);
	void checkFile(const std::string& path);
	std::vector<std::string> listDirectory(std::string path);

public:
	CommandExecutor(): socket(context), resolver(context) {}

	/** Checks the given file or folder. */
	void check(std::string commandLine);

	void nothing(std::string) {}
};

