#include <iostream>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include "../TextCheck/TextCheck.h"
#include "../BinaryCheck/BinaryCheck.h"
#include "safe_queue.h"

using boost::asio::io_context;
using boost::asio::ip::tcp;
io_context context;
tcp::acceptor acceptor(context, tcp::endpoint(tcp::v4(), 8000));
std::mutex mut;
std::condition_variable cv;
safe_queue coada;
std::vector<std::thread *> workerThreads;
bool serverClosing = false;

void process(tcp::socket socket);
void acceptHandler(const boost::system::error_code& code, tcp::socket socket);
bool isInfected(const std::string& content);
void threadWork();

int main()
{
	const int max_connections = 100;
	const int workerThreadCount = 8;
	for (int i = 0; i < workerThreadCount; i++)
		workerThreads.push_back(new std::thread(threadWork));

	boost::system::error_code ec;
	acceptor.listen(max_connections, ec);
	if (ec != boost::system::errc::success)
	{
		std::cout << "Server error.\n";
		return 0;
	}
	acceptor.async_accept(acceptHandler);

	std::thread input([]
	{
		std::string command;
		while (std::getline(std::cin, command))
		{
			if (command == "exit")
			{
				context.stop();
				break;
			}
		}
	});
	io_context::work work(context);
	context.run();

	serverClosing = true;
	cv.notify_all();
	for (std::thread* th : workerThreads)
	{
		th->join();
		delete th;
	}
	input.join();
	std::cout << "Server closing.\n";
	std::cin.get();
}

void acceptHandler(const boost::system::error_code& code, tcp::socket socket)
{
	if (code != boost::system::errc::success)
	{
		std::cout << "Connection error.\n";
		return;
	}
	mut.lock();
	coada.push(new tcp::socket(std::move(socket)));
	mut.unlock();
	cv.notify_one();
	acceptor.async_accept(acceptHandler);
}

void threadWork()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(mut);
		cv.wait(lock, []{ return !coada.empty() || serverClosing; });
		lock.unlock();
		if (serverClosing)
			break;
		tcp::socket *socket = coada.pop();
		process(std::move(*socket));
		delete socket;
	}
}

void process(tcp::socket socket)
{
	boost::system::error_code ec;
	union { char data[sizeof(int)];  int size; } header;
	header.size = 1;
	const std::string clean = "clean";
	const std::string infected = "infected";
	while (header.size > 0)
	{
		boost::asio::read(socket, boost::asio::buffer(header.data), boost::asio::transfer_exactly(sizeof(int)), ec);
		char *buffer = new char[header.size + 1];
		buffer[boost::asio::read(socket, boost::asio::buffer(buffer, header.size), boost::asio::transfer_exactly(header.size), ec)] = 0;

		std::string result;
		if (isInfected(buffer))
			result = infected;
		else
			result = clean;
		header.size = result.length() + 1;
		boost::asio::write(socket, boost::asio::buffer(header.data, sizeof(int)), ec);
		boost::asio::write(socket, boost::asio::buffer(result.c_str(), header.size), ec); // including the null terminator
		boost::asio::read(socket, boost::asio::buffer(header.data, sizeof(int)), boost::asio::transfer_exactly(sizeof(int)), ec);
		delete[] buffer;
	}
	socket.close();
}

bool isInfected(const std::string& content)
{
	std::vector<std::string> textDictionary;
	std::vector<std::pair<const char*, int>> binaryDictionary;
	textDictionary.push_back("make_pair");
	binaryDictionary.push_back(std::make_pair("make_pair", 3));

	return textCheck(content, textDictionary) || binaryCheck(content, binaryDictionary);
}
