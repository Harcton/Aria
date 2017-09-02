#pragma once
#include <mutex>

namespace codex
{
	template<typename T>
	class RAIIVariableSetter
	{
	public:
		RAIIVariableSetter(T& variable, T setValue)
			: variableReference(variable)
			, initialValue(variable)
		{
			variableReference = setValue;
		}
		~RAIIVariableSetter()
		{
			variableReference = initialValue;
		}
		T& variableReference;
	private:
		const T initialValue;
	};

	template<typename T, typename MutexType>
	class RAIIMutexVariableSetter
	{
	public:
		RAIIMutexVariableSetter(T& variable, T setValue, MutexType& mtx)
			: variableReference(variable)
			, initialValue(variable)
			, mutex(mtx)
		{
			std::lock_guard<MutexType> lock(mutex);
			variableReference = setValue;
		}
		~RAIIMutexVariableSetter()
		{
			std::lock_guard<MutexType> lock(mutex);
			variableReference = initialValue;
		}
		T& variableReference;
	private:
		const T initialValue;
		MutexType& mutex;
	};
}