#ifndef LINE_PARSER_H_
#define LINE_PARSER_H_

#include <computer_club.hpp>
#include <cstdint>
#include <string>
#include <vector>

bool is_positive_int(const std::string& str);
bool is_name_correct(const std::string& str);

std::vector<std::string> split_str(std::string str, std::string delimiter);

int64_t get_time_minutes(std::string);
std::string get_time_str(int64_t minutes);

void print_exit(const std::string&);

int64_t parse_positive_number(const std::string&);     // First and third lines
WorkingHours parse_working_hours(const std::string&);  // Second line
Event parse_event(const std::string&);                 // Event

#endif