#include "CommandExecutor.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <windows.h>
#include <shlwapi.h>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>

const std::string CommandExecutor::serverAddress = "127.0.0.1";
const std::string CommandExecutor::serverPort = "8000";

void CommandExecutor::check(std::string commandLine)
{
	if (commandLine.length() == 0)
	{
		std::cout << "No arguments provided.\n";
		return;
	}

	if (!PathFileExistsA(commandLine.c_str()))
	{
		std::cout << "Item does not exist.\n";
		return;
	}

	const int bufferSize = 1024;
	char path[bufferSize];  path[0] = '\0';
	// Get a clean path, formatted by Windows.
	GetFullPathNameA(commandLine.c_str(), bufferSize, path, 0);

	connectToServerAndCheck(path, GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY);
}

void printError(const boost::system::error_code& ec);

void CommandExecutor::connectToServerAndCheck(std::string path, bool isDirectory)
{
	boost::system::error_code ec;
	auto results = resolver.resolve(serverAddress, serverPort, ec);
	if (ec != boost::system::errc::success)
	{
		std::cerr << "Error connecting to server. Command aborted.\n";
		return;
	}

	auto it = results.begin();
	for (; it != results.end(); it++)
	{
		socket.connect(it->endpoint(), ec);
		if (ec == boost::system::errc::success)
			break;
		else
		{
			std::cerr << "Error connecting to server : ";
			printError(ec);
		}
	}
	if (it == results.end())
	{
		std::cerr << "Error connecting to server. Command aborted.\n";
		return;
	}
	std::cout << "Connected to server - sending files :\n";
	std::cout.flush();

	if (isDirectory)
	{
		std::cout << "Acquiring directory :\n";
		std::cout.flush();
		std::vector<std::string> vec = listDirectory(path);
		union { char data[sizeof(int)];  int size; } header;
		header.size = 1;
		auto it = vec.begin();
		for (; it != vec.end() - 1; it++)
		{
			checkFile(*it);
			boost::asio::write(socket, boost::asio::buffer(header.data, sizeof(int)));
		};
		header.size = 0;
		checkFile(*it);
		boost::asio::write(socket, boost::asio::buffer(header.data, sizeof(int)));
	}
	else
	{
		union { char data[sizeof(int)];  int size; } header;
		header.size = 0;
		char filePath[MAX_PATH];
		GetFullPathNameA(path.c_str(), MAX_PATH, filePath, 0);
		checkFile(filePath);
		boost::asio::write(socket, boost::asio::buffer(header.data, sizeof(int)));
	}

	socket.close(ec);
}

bool CommandExecutor::checkIfDefaultFile(const char *name)
{
	return (strcmp(name, ".") == 0 || strcmp(name, "..") == 0);
}

void CommandExecutor::checkFile(const std::string& path)
{
	// Print the file being checked
	std::cout << path << ':';

	std::ifstream ii(path);
	std::string fileContents(std::istreambuf_iterator<char>(ii), {});

	// Send the file
	boost::system::error_code ec;
	union { char data[sizeof(int)];  int size; } header;
	header.size = fileContents.length() + 1;
	boost::asio::write(socket, boost::asio::buffer(header.data, sizeof(int)), ec);
	if (ec != boost::system::errc::success)
		printError(ec);
	boost::asio::write(socket, boost::asio::buffer(fileContents.c_str(), fileContents.length() + 1), ec); // including the null terminator
	if (ec != boost::system::errc::success)
		printError(ec);

	// Receive response
	char bufferc[24];
	boost::asio::read(socket, boost::asio::buffer(header.data), boost::asio::transfer_exactly(sizeof(int)), ec);
	boost::asio::read(socket, boost::asio::buffer(bufferc), boost::asio::transfer_exactly(header.size), ec);
	if (ec != boost::system::errc::success)
		printError(ec);
	std::cout << ' ' << bufferc << '\n';
}

std::vector<std::string> CommandExecutor::listDirectory(std::string path)
{
	std::vector<std::string> results;
	WIN32_FIND_DATAA findData;
	HANDLE handle = INVALID_HANDLE_VALUE;
	char directory[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, directory);
	SetCurrentDirectoryA(path.c_str());

	handle = FindFirstFileA("./*", &findData);
	if (handle == INVALID_HANDLE_VALUE)
	{
		std::cout << "Error reading " << path << ".\n";
		return std::vector<std::string>();
	}
	do
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!checkIfDefaultFile(findData.cFileName))
			{
				std::vector<std::string> subfolder = listDirectory(findData.cFileName);
				results.insert(results.end(), subfolder.begin(), subfolder.end());
			}
		}
		else
		{
			char filePath[MAX_PATH];
			GetFullPathNameA(findData.cFileName, MAX_PATH, filePath, 0);
			results.push_back(filePath);
		}
	}
	while (FindNextFileA(handle, &findData) != 0);
	
	if (GetLastError() != ERROR_NO_MORE_FILES)
		std::cout << "Error reading " << path << ".\n";

	FindClose(handle);

	SetCurrentDirectoryA(directory);

	return results;
}
#define check(ec, s) { if (ec == s) std::cout << #s << '\n'; }
void printError(const boost::system::error_code& ec)
{
	check(ec, boost::system::errc::address_family_not_supported);
	check(ec, boost::system::errc::address_in_use);
	check(ec, boost::system::errc::address_not_available);
	check(ec, boost::system::errc::already_connected);
	check(ec, boost::system::errc::argument_list_too_long);
	check(ec, boost::system::errc::argument_out_of_domain);
	check(ec, boost::system::errc::bad_address);
	check(ec, boost::system::errc::bad_file_descriptor);
	check(ec, boost::system::errc::bad_message);
	check(ec, boost::system::errc::broken_pipe);
	check(ec, boost::system::errc::connection_aborted);
	check(ec, boost::system::errc::connection_already_in_progress);
	check(ec, boost::system::errc::connection_refused);
	check(ec, boost::system::errc::connection_reset);
	check(ec, boost::system::errc::cross_device_link);
	check(ec, boost::system::errc::destination_address_required);
	check(ec, boost::system::errc::device_or_resource_busy);
	check(ec, boost::system::errc::directory_not_empty);
	check(ec, boost::system::errc::executable_format_error);
	check(ec, boost::system::errc::filename_too_long);
	check(ec, boost::system::errc::file_exists);
	check(ec, boost::system::errc::file_too_large);
	check(ec, boost::system::errc::function_not_supported);
	check(ec, boost::system::errc::host_unreachable);
	check(ec, boost::system::errc::identifier_removed);
	check(ec, boost::system::errc::illegal_byte_sequence);
	check(ec, boost::system::errc::inappropriate_io_control_operation);
	check(ec, boost::system::errc::interrupted);
	check(ec, boost::system::errc::invalid_argument);
	check(ec, boost::system::errc::invalid_seek);
	check(ec, boost::system::errc::io_error);
	check(ec, boost::system::errc::is_a_directory);
	check(ec, boost::system::errc::message_size);
	check(ec, boost::system::errc::network_down);
	check(ec, boost::system::errc::network_reset);
	check(ec, boost::system::errc::network_unreachable);
	check(ec, boost::system::errc::not_a_directory);
	check(ec, boost::system::errc::not_a_socket);
	check(ec, boost::system::errc::not_a_stream);
	check(ec, boost::system::errc::not_connected);
	check(ec, boost::system::errc::not_enough_memory);
	check(ec, boost::system::errc::not_supported);
	check(ec, boost::system::errc::no_buffer_space);
	check(ec, boost::system::errc::no_child_process);
	check(ec, boost::system::errc::no_link);
	check(ec, boost::system::errc::no_lock_available);
	check(ec, boost::system::errc::no_message);
	check(ec, boost::system::errc::no_message_available);
	check(ec, boost::system::errc::no_protocol_option);
	check(ec, boost::system::errc::no_space_on_device);
	check(ec, boost::system::errc::no_stream_resources);
	check(ec, boost::system::errc::no_such_device);
	check(ec, boost::system::errc::no_such_device_or_address);
	check(ec, boost::system::errc::no_such_file_or_directory);
	check(ec, boost::system::errc::no_such_process);
	check(ec, boost::system::errc::operation_canceled);
	check(ec, boost::system::errc::operation_in_progress);
	check(ec, boost::system::errc::operation_not_permitted);
	check(ec, boost::system::errc::operation_not_supported);
	check(ec, boost::system::errc::operation_would_block);
	check(ec, boost::system::errc::owner_dead);
	check(ec, boost::system::errc::permission_denied);
	check(ec, boost::system::errc::protocol_error);
	check(ec, boost::system::errc::protocol_not_supported);
	check(ec, boost::system::errc::read_only_file_system);
	check(ec, boost::system::errc::resource_deadlock_would_occur);
	check(ec, boost::system::errc::resource_unavailable_try_again);
	check(ec, boost::system::errc::result_out_of_range);
	check(ec, boost::system::errc::state_not_recoverable);
	check(ec, boost::system::errc::stream_timeout);
	check(ec, boost::system::errc::success);
	check(ec, boost::system::errc::text_file_busy);
	check(ec, boost::system::errc::timed_out);
	check(ec, boost::system::errc::too_many_files_open);
	check(ec, boost::system::errc::too_many_files_open_in_system);
	check(ec, boost::system::errc::too_many_links);
	check(ec, boost::system::errc::too_many_symbolic_link_levels);
	check(ec, boost::system::errc::value_too_large);
	check(ec, boost::system::errc::wrong_protocol_type);
}