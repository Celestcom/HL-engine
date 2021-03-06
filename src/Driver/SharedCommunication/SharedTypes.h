#pragma once

#include <stdint.h>
#include <ctime>
#include "better_enum.h"
namespace NullSpace {
	namespace SharedMemory {


		struct Quaternion {
			Quaternion() : x(0), y(0), z(0), w(0) {}
			Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
			
			float x;
			float y;
			float z;
			float w;


		};


		struct Vector3 {
			Vector3() : x(0), y(0), z(0) {}
			Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
		
			float x;
			float y;
			float z;
		};

		struct TrackingData {
			static TrackingData withQuat(uint32_t region, Quaternion quat) {
				return TrackingData{ region, quat, {}, {} };
			}

			static TrackingData withCompass(uint32_t region, Vector3 compass) {
				return TrackingData{ region, {}, compass, {} };
			}

			static TrackingData withGrav(uint32_t region, Vector3 grav) {
				return TrackingData{ region, {}, {}, grav };
			}
			uint32_t region;
			Quaternion quat;
			Vector3 compass;
			Vector3 gravity;
		};

		struct TrackingUpdate {
			Quaternion chest;
			Quaternion left_upper_arm;
			Quaternion right_upper_arm;

			Vector3 chest_compass;
			Vector3 chest_gravity;

			Vector3 left_upper_arm_compass;
			Vector3 left_upper_arm_gravity;

			Vector3 right_upper_arm_compass;
			Vector3 right_upper_arm_gravity;
		};

		enum DeviceStatus { Unknown, Connected, Disconnected };
		
		
		constexpr int MAX_DEVICES_PER_SYSTEM = 16;

	

		struct DeviceInfo {
			uint32_t Id; //0
			uint32_t Concept;
			char DeviceName[128]; //Hardlight Mk III Suit
			DeviceStatus Status;

		};

		struct NodeInfo {
			uint32_t Id;
			uint32_t DeviceId;
			char NodeName[128];
			uint32_t Region;
			uint32_t Type;
		};

		
	
		struct ServiceInfo {
			int MajorVersion;
			int MinorVersion;
			int PatchVersion;

		};

		struct SentinelObject {
			ServiceInfo Info;
			std::time_t TimeStamp;
		};

		static const TrackingUpdate NullTrackingUpdate = {};


		struct Color {
			float r;
			float g;
			float b;
			float a;
		};

		struct Data {
			float data_0;
			float data_1;
			float data_2;
			float data_3;
		};
		struct Intensity {
			float intensity;
		};

		enum class RegionPairType {
			Unknown = 0,
			Color = 1,
			Intensity = 2
		};
		struct RegionPair {
			uint32_t Region;
			uint32_t Type;
			uint32_t Id;
			Data Value;


		};

		inline bool operator==(const RegionPair& lhs, const RegionPair& rhs) {
			return lhs.Id == rhs.Id;
		}



		constexpr uint32_t SUBREGION_BLOCK = 1000000;

		BETTER_ENUM(nsvr_shared_region, uint32_t,
			identifier_unknown,
			identifier_body = 1 * SUBREGION_BLOCK,
			identifier_torso = 2 * SUBREGION_BLOCK,
			identifier_torso_front = 3 * SUBREGION_BLOCK,
			identifier_middle_sternum = 3 * SUBREGION_BLOCK + 1,

			identifier_chest_left = 4 * SUBREGION_BLOCK,
			identifier_chest_right = 5 * SUBREGION_BLOCK,
			identifier_upper_ab_left = 6 * SUBREGION_BLOCK,
			identifier_middle_ab_left = 7 * SUBREGION_BLOCK,
			identifier_lower_ab_left = 8 * SUBREGION_BLOCK,
			identifier_upper_ab_right = 9 * SUBREGION_BLOCK,
			identifier_middle_ab_right = 10 * SUBREGION_BLOCK,
			identifier_lower_ab_right = 11 * SUBREGION_BLOCK,
			identifier_torso_back = 12 * SUBREGION_BLOCK,
			identifier_torso_left = 13 * SUBREGION_BLOCK,
			identifier_torso_right = 14 * SUBREGION_BLOCK,
			identifier_upper_back_left = 15 * SUBREGION_BLOCK,
			identifier_upper_back_right = 16 * SUBREGION_BLOCK,
			identifier_upper_arm_left = 17 * SUBREGION_BLOCK,
			identifier_lower_arm_left = 18 * SUBREGION_BLOCK,
			identifier_upper_arm_right = 19 * SUBREGION_BLOCK,
			identifier_lower_arm_right = 20 * SUBREGION_BLOCK,
			identifier_shoulder_left = 21 * SUBREGION_BLOCK,
			identifier_shoulder_right = 22 * SUBREGION_BLOCK,
			identifier_upper_leg_left = 23 * SUBREGION_BLOCK,
			identifier_lower_leg_left = 24 * SUBREGION_BLOCK,
			identifier_upper_leg_right = 25 * SUBREGION_BLOCK,
			identifier_lower_leg_right = 26 * SUBREGION_BLOCK,
			identifier_head = 27 * SUBREGION_BLOCK,
			identifier_palm_left = 28 * SUBREGION_BLOCK,
			identifier_palm_right = 29 * SUBREGION_BLOCK);

	}
}



