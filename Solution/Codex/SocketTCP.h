#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>
#include <type_traits>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

#include "Protocol.h"
#include "IOService.h"

namespace codex
{
	namespace protocol
	{
		class WriteBuffer;
		class ReadBuffer;
	}

	class SocketTCP
	{
	private:
		typedef uint32_t ExpectedBytesType;
	public:

		/* If a shared IOService pointer is provided, the socket uses that IOService. Otherwise, creates its own IOSerice. */
		
		SocketTCP(IOService& ioService);
		virtual ~SocketTCP();
		
		/* Perform a synchronous connection attempt. */
		bool connect(const char* address_ipv4, const uint16_t port);

		/* Disconnect from the currently connected endpoint. */
		void disconnect();

		/* Sends buffer to the connected endpoint. */
		bool sendPacket(const protocol::WriteBuffer& buffer);

		/* Returns false if the memory allocation fails, or the socket is currently receiving data. */
		bool resizeReceiveBuffer(const size_t newSize);

		/* Starts receiving data from the connected endpoint. Non-blocking call. Callback return value specifies whether to keep receiving. */
		bool startReceiving(const std::function<bool(codex::protocol::ReadBuffer&)> onReceiveCallback);

		/* Starts listening for a new incoming connection. Upon success, a connection is made. Non-blocking call. */
		bool startAccepting(const uint16_t port, const std::function<void(SocketTCP&)> onAcceptCallback);

		/* Stops receiving data. */
		void stopReceiving();

		/* Stops accepting an incoming connection. */
		void stopAccepting();

		/* Returns the address of the remotely connected socket. */
		std::string getRemoteAddress();

		/* Returns the port of the remotely connected socket. */
		uint16_t getRemotePort();

		bool isAccepting() const { std::lock_guard<std::recursive_mutex> locks(mutex); return acceptor != nullptr; }
		bool isReceiving() const { std::lock_guard<std::recursive_mutex> locks(mutex); return receiving; }
		bool isConnected() const { std::lock_guard<std::recursive_mutex> locks(mutex); return connected; }

	protected:

		/* Blocks until receiving has stopped. */
		bool waitUntilFinishedReceiving();

		/* Returns true if finished receiving within the given timeout. Returns false if receiving didn't stop within the given time. */
		bool waitUntilFinishedReceiving(uint64_t millisecondTimeout);
		
		void onAccept(const boost::system::error_code error);
		bool handshakeReceiveHandler(codex::protocol::ReadBuffer& buffer);
		void receiveHandler(const boost::system::error_code& error, std::size_t bytes);

		mutable std::recursive_mutex mutex;
		IOService& ioService;
		boost::asio::ip::tcp::socket socket;
		boost::asio::ip::tcp::acceptor* acceptor;
		std::function<bool(codex::protocol::ReadBuffer&)> onReceiveCallback;
		std::function<void(SocketTCP&)> onAcceptCallback;
		ExpectedBytesType expectedBytes;
		std::vector<unsigned char> receiveBuffer;
		bool receiving;
		bool connected;
		bool reverseByteOrdering;//If set to true, the socket will automatically set the read buffer byte ordering setting, and will prompt if incorrecly set write buffer is sent.

	};

	class ShellSocketTCP : public SocketTCP
	{
	public:

		ShellSocketTCP(IOService& ioService);
		~ShellSocketTCP() override;

		/*
			Sends a ghost request to the remote endpoint, defined by the codex protocol.
		*/
		bool requestGhost(const std::string& ghostName);

	private:

		bool internalReceiveHandler(codex::protocol::ReadBuffer& buffer);

		std::mutex requestGhostMutex;
		std::atomic<bool> requestGhostResponseReceived;
		uint64_t requestGhostReceivedResponse;
		std::string requestGhostReceivedResponseAddress;
		uint16_t requestGhostReceivedResponsePort;

	};

#ifndef SHELL_CODEX
	class AriaSocketTCP : public SocketTCP
	{
	public:

		AriaSocketTCP(IOService& ioService);
		~AriaSocketTCP() override;

		/*
		Receive handler for expected ghost requests.
		*/
		bool ghostRequestHandler(codex::protocol::ReadBuffer& buffer);

	private:
		std::string ghostDirectory;
	};
#endif
}