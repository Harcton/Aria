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
					buffer.write(distance);
					buffer.write(runSteerServo);
					buffer.write(runDCMotor);
					buffer.write(runDistanceSensor);
				}
				void read(protocol::ReadBuffer& buffer)
				{
					buffer.read(steerAngle);
					buffer.read(dcMotorStrength);
					buffer.read(distance);
					buffer.read(runSteerServo);
					buffer.read(runDCMotor);
					buffer.read(runDistanceSensor);
				}
				float steerAngle;
				float dcMotorStrength;
				float distance;
				bool runSteerServo;
				bool runDCMotor;
				bool runDistanceSensor;
			};

			/* Sent by ghost */
			struct GhostNetState
			{
				void write(protocol::WriteBuffer& buffer)
				{
					buffer.write(steerAngle);
					buffer.write(dcMotorStrength);
					buffer.write(distance);
					buffer.write(runSteerServo);
					buffer.write(runDCMotor);
					buffer.write(runDistanceSensor);
				}
				void read(protocol::ReadBuffer& buffer)
				{
					buffer.read(steerAngle);
					buffer.read(dcMotorStrength);
					buffer.read(distance);
					buffer.read(runSteerServo);
					buffer.read(runDCMotor);
					buffer.read(runDistanceSensor);
				}
				float steerAngle;
				float dcMotorStrength;
				float distance;
				bool runSteerServo;
				bool runDCMotor;
				bool runDistanceSensor;
			};

			
		}
	}
}