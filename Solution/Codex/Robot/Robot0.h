#pragma once
#include <stdint.h>
#include "Protocol.h"

namespace codex
{
	namespace robot
	{
		namespace robot0
		{
			enum class PacketType : uint8_t
			{
				update,
			};

			/* Sent by shell */
			struct ShellNetState
			{
				void write(protocol::WriteBuffer& buffer)
				{
					buffer.write(steerAngle);
					buffer.write(dcMotorStrength);
					buffer.write(runSteerServo);
					buffer.write(runDCMotor);
				}
				void read(protocol::ReadBuffer& buffer)
				{
					buffer.read(steerAngle);
					buffer.read(dcMotorStrength);
					buffer.read(runSteerServo);
					buffer.read(runDCMotor);
				}
				float steerAngle;
				float dcMotorStrength;
				bool runSteerServo;
				bool runDCMotor;
			};

			/* Sent by ghost */
			struct GhostNetState
			{
				void write(protocol::WriteBuffer& buffer)
				{
					buffer.write(steerAngle);
					buffer.write(dcMotorStrength);
					buffer.write(runSteerServo);
					buffer.write(runDCMotor);
				}
				void read(protocol::ReadBuffer& buffer)
				{
					buffer.read(steerAngle);
					buffer.read(dcMotorStrength);
					buffer.read(runSteerServo);
					buffer.read(runDCMotor);
				}
				float steerAngle;
				float dcMotorStrength;
				bool runSteerServo;
				bool runDCMotor;
			};

			
		}
	}
}