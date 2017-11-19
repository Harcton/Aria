#include "SyncType.h"
#include "SyncManager.h"
#include "CodexAssert.h"
#include "Protocol.h"

namespace codex
{
	const SyncType::TypeId::ValueType SyncType::TypeId::invalidValue = std::numeric_limits<SyncType::TypeId::ValueType>::max();
	SyncType::TypeId::TypeId()
		: value(invalidValue)
	{
	}

	SyncType::TypeId::TypeId(const uint16_t val)
		: value(val)
	{
	}

	SyncType::~SyncType()
	{
		if (syncManager)
		{
			//Going out of scope -> severe the link to the sync manager
			syncManager->removeEntry(*this);
			syncManager = nullptr;
		}
	}

	SyncType::TypeId& SyncType::TypeId::operator=(const TypeId& other)
	{
		value = other.value;
		return *this;
	}

	SyncType::TypeId::operator bool() const
	{
		return value != invalidValue;
	}

	SyncType::TypeId::operator SyncType::TypeId::ValueType() const
	{
		return value;
	}

	void SyncType::TypeId::write(protocol::WriteBuffer& buffer)
	{
		buffer.write((uint16_t)value);
	}

	void SyncType::TypeId::read(protocol::ReadBuffer& buffer)
	{
		buffer.read((uint16_t&)value);
	}

	SyncType::SyncType()
		: syncManager(nullptr)
	{
	}

	SyncType::SyncType(const SyncType& other)
		: syncManager(nullptr)
	{
		CODEX_ASSERT(false);//What to do?
	}

	SyncType::SyncType(const SyncType&& other)
		: syncManager(nullptr)
	{
		CODEX_ASSERT(false);//What to do?
	}

	SyncType& SyncType::operator=(const SyncType& other)
	{
		CODEX_ASSERT(false);//What to do?
		return *this;
	}

	SyncType& SyncType::operator=(const SyncType&& other)
	{
		CODEX_ASSERT(false);//What to do?
		return *this;
	}
}

#define SYNCTYPE_DECL(MyClass) \
public: \
void syncCreate(codex::protocol::WriteBuffer& buffer) override; \
void syncCreate(codex::protocol::ReadBuffer& buffer) override; \
void syncUpdate(codex::protocol::WriteBuffer& buffer) override; \
void syncUpdate(codex::protocol::ReadBuffer& buffer) override; \
void syncRemove(codex::protocol::WriteBuffer& buffer) override; \
void syncRemove(codex::protocol::ReadBuffer& buffer) override; \
private: \
static codex::SyncType::TypeId syncTypeId; \
static const std::string syncTypeName;
#define SYNCTYPE_IMPL(MyClass) \
codex::SyncType::TypeId MyClass::syncTypeId = std::numeric_limits<codex::SyncType::TypeId>::max(); \
const std::string MyClass::syncTypeName = #MyClass;