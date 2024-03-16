#include <iostream>
#include <string>
#include "DatabaseAccess.h"
#include "AlbumManager.h"

#include <ctime>
#include <chrono>

int getCommandNumberFromUser()
{
	std::string message("\nPlease enter any command(use number): ");
	std::string numericStr("0123456789");

	std::cout << message << std::endl;
	std::string input;
	std::getline(std::cin, input);

	while (std::cin.fail() || std::cin.eof() || input.find_first_not_of(numericStr) != std::string::npos) {

		std::cout << "Please enter a number only!" << std::endl;

		if (input.find_first_not_of(numericStr) == std::string::npos) {
			std::cin.clear();
		}

		std::cout << std::endl << message << std::endl;
		std::getline(std::cin, input);
	}

	return std::atoi(input.c_str());
}

void printWelcome()
{
	auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	std::cout << "Gallery project by Itay Dali" << std::endl << ctime(&timenow);
	std::cout << "===================" << std::endl;
	std::cout << "Type " << HELP << " to a list of all supported commands" << std::endl;
}

int main(void)
{
	// initialization data access
	MemoryAccess dataAccess;

	// initialize album manager
	AlbumManager albumManager(dataAccess);


	std::string albumName;

	printWelcome();

	do {
		int commandNumber = getCommandNumberFromUser();

		try {
			albumManager.executeCommand(static_cast<CommandType>(commandNumber));
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}
	} while (true);
}


