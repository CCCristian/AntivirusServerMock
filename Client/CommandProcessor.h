#include <cstring>
#include <string>
#include <vector>
#include <map>
#include "CommandExecutor.h"

class CommandProcessor
{
	struct Commands
	{
		static const int CHECK = 0, EXIT = 1, Count = 2;
	};
	static const char* commandsTexts[Commands::Count];
	static const char* commandsParameters[Commands::Count];
	CommandExecutor executor;

public:
	CommandProcessor() = default;

	/** Return false if an exit is requested. */
	bool processCommand(std::string commandString);
	static void printInfo();
};
