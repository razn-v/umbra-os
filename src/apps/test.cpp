#include <dirent.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>

void ls() {
    char cwd[256];
    getcwd(cwd, 256);
    DIR* directory = opendir(cwd);

    if (directory == NULL) {
        std::cerr << "Unable to open directory" << std::endl;
        return;
    }

    // Read and print file names from the directory
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_type == DT_DIR) {
            std::cout << entry->d_name << "/" << std::endl;
        } else if (entry->d_type == DT_REG) {
            std::cout << entry->d_name << std::endl;
        }
    }
}

void cd(std::vector<std::string> args) {
    if (args.size() == 0) {
        return;
    }

    std::string path = args.at(0);
    chdir(path.c_str());
}

int main(int argc, char** argv) {
    std::cout << std::endl << "Welcome to UmbraOS!" << std::endl << std::endl;

    while (true) {
        std::cout << "$ ";

        std::string input;
        std::getline(std::cin, input);

        std::istringstream iss(input);
        std::vector<std::string> arguments;

        while (iss) {
            std::string arg;
            iss >> arg;
            if (!arg.empty()) {
                arguments.push_back(arg);
            }
        }

        if (!arguments.empty()) {
            // The first argument is the command itself
            std::string command = arguments[0];

            // The rest of the arguments are the command's arguments
            std::vector<std::string> cmd_args(arguments.begin() + 1, arguments.end());

            if (command == "ls") {
                ls();
            } else if (command == "cd") {
                cd(cmd_args);
            } else {
                std::cout << "Invalid command (" << command << ")!" << std::endl;
            }
        }
    }

    return 0;
}
