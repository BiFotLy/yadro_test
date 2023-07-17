#include <computer_club.hpp>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << "Error: first argument needs to be \"path/to/file\""
                  << std::endl;
        std::exit(EXIT_FAILURE);
    } else if (argc != 2) {
        std::cout << "Error: too many arguments" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::string filename(argv[1]);
    ComputerClub comp_club;
    comp_club.parse_file(filename);

    std::cout << comp_club.get_fmt_results();

    std::exit(EXIT_SUCCESS);
}