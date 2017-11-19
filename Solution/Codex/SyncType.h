#pragma once
#include <stdint.h>

namespace codex
{
	namespace protocol
	{
		class WriteBuffer;
		class ReadBuffer;
	}

	/*
		Synchronizes states from ghost to shell or vice versa.
		Synchronizable types must implement the following methods:
		syncCreate(WriteBuffer/ReadBuffer)
		contains the initial state data before the type begins the updates.
		syncUpdate(WriteBuffer/ReadBuffer)
		contains update data that is sent between set intervals or when sync is specifically requested.
		syncRemove(WriteBuffer/ReadBuffer)
		contains the end state data after the updates have been stopped.

		Sync types are created and controlled outside the sync manager.
		They are then connected to the sync manager, after which they start to exist on a remote sync manager as well.
		This remote sync manager contains the type instance within.
	*/
	class SyncManager;
	class SyncType
	{
	public:
		class TypeId
		{
		public:
			typedef uint16_t ValueType;
		public:
			TypeId();
			TypeId(const uint16_t val);
			TypeId& operator=(const TypeId& other);
			operator ValueType() const;
			operator bool() const;
			void write(protocol::WriteBuffer& buffer);
			void read(protocol::ReadBuffer& buffer);

		private:
			static const ValueType invalidValue;
			ValueType value;
		};

	public:
		SyncType();
		SyncType(const SyncType& other);
		SyncType(const SyncType&& other);
		SyncType& operator=(const SyncType& other);
		SyncType& operator=(const SyncType&& other);
		virtual ~SyncType();

		virtual TypeId getTypeId() const = 0;
		virtual TypeId setTypeId(const TypeId) = 0;

		virtual void syncCreate(protocol::WriteBuffer& buffer) {}
		virtual void syncCreate(protocol::ReadBuffer& buffer) {}
		virtual void syncUpdate(protocol::WriteBuffer& buffer) {}
		virtual void syncUpdate(protocol::ReadBuffer& buffer) {}
		virtual void syncRemove(protocol::WriteBuffer& buffer) {}
		virtual void syncRemove(protocol::ReadBuffer& buffer) {}

	private:
		friend class SyncManager;
		SyncManager* syncManager;
	};
}

#define SYNCTYPE_DECL(MyClass) \
private: \
static codex::SyncType::TypeId syncTypeId; \
static const std::string syncTypeName;
#define SYNCTYPE_IMPL(MyClass) \
codex::SyncType::TypeId MyClass::syncTypeId = std::numeric_limits<codex::SyncType::TypeId>::max(); \
const std::string MyClass::syncTypeName = #MyClass;