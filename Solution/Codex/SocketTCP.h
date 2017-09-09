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
#include "CodexTime.h"

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
		bool connect(const protocol::AddressType& address_ipv4, const protocol::PortType& port);
		bool connect(const protocol::Endpoint& endpoint);

		/* Disconnect from the currently connected endpoint. */
		void disconnect();

		/* Sends buffer to the connected endpoint. Codex-level packet type specification is also possible (only for advanced use). */
		bool sendPacket(const protocol::WriteBuffer& buffer, const protocol::PacketType packetType = protocol::PacketType::undefined);

		/* Returns false if the memory allocation fails, or the socket is currently receiving data. */
		bool resizeReceiveBuffer(const size_t newSize);

		/* Starts receiving data from the connected endpoint. Non-blocking call. Callback return value specifies whether to keep receiving. */
		bool startReceiving(const std::function<bool(codex::protocol::ReadBuffer&)> onReceiveCallback);

		/* Starts listening for a new incoming connection. Upon success, a connection is made. Non-blocking call. Callback is called even if no connection is made! */
		bool startAccepting(const protocol::PortType port, const std::function<void(SocketTCP&)> onAcceptCallback);

		/* Stops receiving data. */
		void stopReceiving();

		/* Stops accepting an incoming connection. */
		void stopAccepting();

		/* Remote endpoint. */
		protocol::AddressType getRemoteAddress() const;
		protocol::PortType getRemotePort() const;
		protocol::Endpoint getRemoteEndpoint() const;
		
#ifdef GHOST_CODEX
		/* This is the byte ordering(endianness) that all passed write buffers should comply. */
		protocol::Endianness getRemoteEndianness() const;
#endif
		/* Returns true if the socket is currently listening for an incoming connection on a port, or the connection is currently being established. */
		bool isAccepting() const;
		/* Returns true if socket is currently able to receive incoming packets. */
		bool isReceiving() const;
		/* Returns true if the socket is currently connected to a remote codex socket. */
		bool isConnected() const;
		
		/* Prevents other threads from using this socket. Incoming receive/accept callbacks will have to wait until the thread lock is released. */
		void enableThreadLock() { mutex.lock(); }
		/* Releases previously enabled thread lock. */
		void releaseThreadLock() { mutex.unlock(); }

	protected:
		
		/* Disconnect the socket with the specified type */
		void disconnect(const protocol::DisconnectType disconnectType);

		/* Blocks until receiving has stopped. */
		void waitUntilFinishedReceiving();

		/* Blocks until accepting has stopped. */
		void waitUntilFinishedAccepting();
		
		/* Blocks until handshakeReceived==true, or time is out. */
		void waitUntilReceivedHandshake(const time::TimeType timeout);

		/* Boost acceptor calls this method when an incoming connection is being accepted. If no error is detected, launches the synchronous codexAccept method in a different thread (codexAcceptThread). */
		void onAccept(const boost::system::error_code error);
		/* Asynchronous method for running the handshake procedure with the remote endpoint. */
		void codexAccept();

		//Receive handlers
		void receiveHandler(const boost::system::error_code& error, std::size_t bytes);//Boost initially passes received data to this receive handler.
		bool codexReceiveHandler(codex::protocol::ReadBuffer& buffer);//Internal receive handler, unpacks codex header. Calls the user defined receive handler.

		mutable std::recursive_mutex mutex;
		IOService& ioService;
		boost::asio::ip::tcp::socket socket;
		boost::asio::ip::tcp::acceptor* acceptor;
		std::thread* codexAcceptThread;
		std::function<bool(codex::protocol::ReadBuffer&)> onReceiveCallback;//User defined receive handler
		std::function<void(SocketTCP&)> onAcceptCallback;
		ExpectedBytesType expectedBytes;
		std::vector<unsigned char> receiveBuffer;
		codex::time::TimeType lastReceiveTime;
		bool receiving;
		bool accepting;
		bool connected;
		bool connecting;//Set to true for the duration of connect attempt
		bool handshakeSent;//Refers to the current connection
		bool handshakeReceived;//Refers to the current connection
#ifdef GHOST_CODEX
		codex::protocol::Endianness remoteEndianness;
#endif
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

#ifdef GHOST_CODEX
	class GhostSocketTCP : public SocketTCP
	{
	public:

		GhostSocketTCP(IOService& ioService);
		~GhostSocketTCP() override;

		/*
		Receive handler for expected ghost requests.
		*/
		bool ghostRequestHandler(codex::protocol::ReadBuffer& buffer);

	private:
		std::string ghostDirectory;
	};
#endif
}