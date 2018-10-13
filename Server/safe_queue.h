#pragma once

#include <queue>
#include <thread>
#include <boost/asio.hpp>

class safe_queue
{
	std::queue<boost::asio::ip::tcp::socket *> coada;
	std::mutex mut;

public:
	boost::asio::ip::tcp::socket* pop()
	{
		std::lock_guard<std::mutex> lock(mut);
		boost::asio::ip::tcp::socket *sock = coada.front();
		coada.pop();
		return sock;
	}
	void push(boost::asio::ip::tcp::socket* sock)
	{
		std::lock_guard<std::mutex> lock{ mut };
		coada.push(sock);
	}
	bool empty()
	{
		std::lock_guard<std::mutex> lock{ mut };
		return coada.empty();
	}
};