#include "CommandProcessor.h"
#include <iostream>

std::string trim(const std::string& str)
{
	if (str.length() == 0)
		return "";
	int first = str.find_first_not_of(' ');
	if (first < 0)
		return "";
	int last = str.find_last_not_of(' ');
	return str.substr(first, last - first + 1);
}

bool CommandProcessor::processCommand(std::string commandString)
{
	commandString = trim(commandString);
	if (commandString.length() == 0)
		return true;
	int pos = commandString.find_first_of(' ');
	std::string commandType;
	if (pos < 0)
		commandType = commandString;
	else
		commandType = commandString.substr(0, pos);


	if (commandType == "exit")
		return false;

	std::string commandLine = "";
	if (commandString.length() - commandType.length() >= 2)
		commandLine = commandString.substr(commandType.length() + 1);

	if (commandType == commandsTexts[Commands::CHECK])
		executor.check(commandLine);
	else
	{
		std::cout << "Invalid command.\n";
		return true;
	}

	return true;
}

const char* CommandProcessor::commandsTexts[CommandProcessor::Commands::Count] = { "check", "exit" };
const char* CommandProcessor::commandsParameters[CommandProcessor::Commands::Count] = { " <file>/<folder>", "" };
void CommandProcessor::printInfo()
{
	std::cout << "List of commands:\n";
	for (int i = 0; i < Commands::Count; i++)
		std::cout << "> " << commandsTexts[i] << commandsParameters[i] << '\n';
	std::cout << '\n';
}
