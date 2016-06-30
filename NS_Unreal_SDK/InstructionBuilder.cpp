#include "InstructionBuilder.h"
#include <iostream>
#include "HexUtils.h"
#include <fstream>
InstructionBuilder::InstructionBuilder()
{
	std::string validParams[4] = { "zone", "effect", "data", "register" };
	for (std::string param : validParams) {
		paramDict[param] = std::unordered_map<string, uint8_t>();
	}

}
static std::string EnumToString(int s) {
	return "hi";
}


InstructionBuilder::~InstructionBuilder()
{
}


InstructionBuilder& InstructionBuilder::UseInstruction(std::string name) {
	this->parameters.clear();
	this->instruction = name;
	return *this;
}




InstructionBuilder& InstructionBuilder::WithParam(std::string key, std::string val) {
	this->parameters[key] = val;
	return *this;
}

bool InstructionBuilder::Verify() {
	if (instructions.find(this->instruction) == instructions.end()) {
		return false;
	}

	Instruction desired = instructions[this->instruction];
	for (std::string param : desired.Parameters) {
		if (parameters.find(param) == parameters.end()) {
			return false;
		}
		auto dict = this->paramDict[param];
		if (dict.find(parameters[param]) == dict.end()) {
			return false;
		}

	}
	return true;
}


std::string InstructionBuilder::GetDebugString() {
	std::string description = this->instruction + ": ";
	int index = 0;
	for (auto param : this->parameters)
	{
		index++;
		description += param.first + " = " + param.second;
		if (index < this->parameters.size())
		{
			description += ", ";
		}
	}
	return description;
}
Packet InstructionBuilder::Build() {
	Instruction desired = instructions[this->instruction];
	const int packetLength = 7 + this->parameters.size();
	uint8_t* packet = new uint8_t[packetLength];
	std::fill(packet, packet + packetLength, 0);
	packet[0] = 0x24;
	packet[1] = 0x02;
	assert(desired.ByteId <= 255);
	packet[2] = desired.ByteId;
	assert(packetLength <= 255);
	packet[3] = packetLength;

	const std::size_t numParams = this->parameters.size();
	for (std::size_t i = 0; i < numParams; i++) {
		std::string paramKey = desired.Parameters[i];
		std::string userParamVal = this->parameters[paramKey];
		auto paramKeyToByteId = this->paramDict[paramKey];
		uint8_t id = paramKeyToByteId[userParamVal];
		packet[i + 4] = id;
	}

	packet[packetLength - 3] = 0xFF;
	packet[packetLength - 2] = 0xFF;
	packet[packetLength - 1] = 0x0A;
	return Packet(packet, packetLength);
}


bool InstructionBuilder::LoadKeyValue(std::unordered_map<string, uint8_t>& dict, Json::Value json) {
	auto names = json.getMemberNames();
	std::ofstream myfile;
	myfile.open("keyval_" + names[0] + ".txt");

	for (std::string key : names) {
		std::string val = json.get(key, "0x00").asString();
		const char* hexChars = &val.c_str()[2];
		uint8_t hex[1]{ 0 };
		HexStringToInt(hexChars, hex);
		dict[key] = hex[0];
		myfile <<  "(Effect::" + key + ", \""+ key + "\")\n";
	}
	

	
	return false;
}

bool InstructionBuilder::LoadEffects(const Json::Value& json) {
	return LoadKeyValue(this->paramDict["effect"], json);
}

bool InstructionBuilder::LoadZones(const Json::Value& json) {
	return LoadKeyValue(this->paramDict["zone"], json);
}

bool InstructionBuilder::LoadInstructions(const Json::Value& json) {
	std::size_t numInstructions = json.size();
	for (std::size_t i = 0; i < numInstructions; ++i) {
		Instruction inst;
		inst.Deserialize(json[i]);
		instructions[inst.Name] = inst;
	}
	return false;
}

