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
					buffer.write(motionSensorState);
					buffer.write(runSteerServo);
					buffer.write(runDCMotor);
					buffer.write(runDistanceSensor);
					buffer.write(runMotionSensor);
				}
				void read(protocol::ReadBuffer& buffer)
				{
					buffer.read(steerAngle);
					buffer.read(dcMotorStrength);
					buffer.read(distance);
					buffer.read(motionSensorState);
					buffer.read(runSteerServo);
					buffer.read(runDCMotor);
					buffer.read(runDistanceSensor);
					buffer.read(runMotionSensor);
				}
				float steerAngle;
				float dcMotorStrength;
				float distance;
				float motionSensorState;

				bool runSteerServo;
				bool runDCMotor;
				bool runDistanceSensor;
				bool runMotionSensor;
			};

			/* Sent by ghost */
			struct GhostNetState
			{
				void write(protocol::WriteBuffer& buffer)
				{
					buffer.write(steerAngle);
					buffer.write(dcMotorStrength);
					buffer.write(distance);
					buffer.write(motionSensorState);
					buffer.write(runSteerServo);
					buffer.write(runDCMotor);
					buffer.write(runDistanceSensor);
					buffer.write(runMotionSensor);
				}
				void read(protocol::ReadBuffer& buffer)
				{
					buffer.read(steerAngle);
					buffer.read(dcMotorStrength);
					buffer.read(distance);
					buffer.read(motionSensorState);
					buffer.read(runSteerServo);
					buffer.read(runDCMotor);
					buffer.read(runDistanceSensor);
					buffer.read(runMotionSensor);
				}
				float steerAngle;
				float dcMotorStrength;
				float distance;
				float motionSensorState;

				bool runSteerServo = false;
				bool runDCMotor = false;
				bool runDistanceSensor = false;
				bool runMotionSensor = false;
			};

			
		}
	}
}