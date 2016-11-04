#pragma once
#include <unordered_map>
#include "Instruction.h"
#include "json\json.h"

typedef std::function<void(const Json::Value& json)> Loader;


class InstructionLoadingException : public std::exception
{
public:
	InstructionLoadingException(std::string m):_message(m) {

	}

	virtual const char* what() const throw()
	{
		return _message.c_str();
	}
private:
	std::string _message;
};

class InstructionSet
{
public:
	InstructionSet();
	~InstructionSet(); 
	bool LoadAll();
	const std::unordered_map<std::string, Instruction>& Instructions() const;
	const std::unordered_map<std::string, std::unordered_map<std::string, uint8_t>>&  ParamDict() const;
private:
	std::unordered_map<std::string, Instruction> _instructions;
	std::unordered_map<std::string, std::unordered_map<std::string, uint8_t>> _paramDict;
	void LoadKeyValue(std::unordered_map<std::string, uint8_t>& dict, Json::Value json);
	void loadFromJson(std::string path, Loader loader);
	void LoadInstructions(const Json::Value& json);

};

