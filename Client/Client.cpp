#include <iostream>
#include <string>
#include "CommandProcessor.h"
void exitApp()
{
	// Cleanup
}
int main()
{
	CommandProcessor commandProcessor;
	commandProcessor.printInfo();
	std::string commandString;
	std::cout << ">> ";
	while (std::getline(std::cin, commandString))
	{
		if (!commandProcessor.processCommand(commandString))
			break;
		std::cout << "\n>> ";
	}
	std::cout << "Exiting ...\n";
	exitApp();
}
