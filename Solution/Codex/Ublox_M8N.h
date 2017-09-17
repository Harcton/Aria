#pragma once
#include "Codex/Device/ThreadedDevice.h"

namespace codex
{
	namespace device
	{
		class Ublox_M8N : public codex::device::ThreadedDevice
		{
		public:
			Ublox_M8N();
			~Ublox_M8N() override;

			void onStart() override;
			void update() override;
			void onStop() override;

		private:
			mutable std::recursive_mutex mutex;

		};
	}
}