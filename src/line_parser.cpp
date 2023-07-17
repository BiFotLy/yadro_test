#include <computer_club.hpp>
#include <cstdint>
#include <format>
#include <iostream>
#include <line_parser.hpp>
#include <ostream>
#include <regex>
#include <string>
#include <vector>

bool is_positive_int(const std::string& str) {
    std::string::const_iterator it = str.begin();
    while (it != str.end() && std::isdigit(*it)) {
        ++it;
    }
    return !str.empty() && it == str.end();
}

int64_t parse_time(const std::string& str) {
    // Match format "hh:mm".
    std::regex time_regex = std::regex("[0-9]{2}:[0-9]{2}");
    if (!std::regex_match(str, time_regex)) {
        return -1;
    }

    // Calculate total minutes by formula: hours * 60 + minutes.
    int64_t total_minutes = 0;
    auto time_vec = split_str(str, ":");
    total_minutes += std::stoi(time_vec[0]) * 60 + std::stoi(time_vec[1]);

    if (total_minutes >= 24 * 60) {
        return -1;
    }

    return total_minutes;
}

bool is_name_correct(const std::string& str) {
    return std::regex_match(str, std::regex("^[a-z0-9_-]+$"));
}

std::string get_time_str(int64_t minutes) {
    std::string result;
    std::format_to(std::back_inserter(result), "{0:0>2d}:{1:0>2d}",
                   minutes / 60, minutes % 60);
    return result;
}

std::vector<std::string> split_str(std::string str, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> result;

    while ((pos_end = str.find(delimiter, pos_start)) != std::string::npos) {
        token = str.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        result.push_back(token);
    }

    result.push_back(str.substr(pos_start));
    return result;
}

void print_exit(const std::string& line) {
    std::cout << line << std::endl;
    std::exit(EXIT_FAILURE);
}

// First and third lines.
int64_t parse_positive_number(const std::string& line) {
    if (!is_positive_int(line) || line[0] == '0') {
        print_exit(line);
    }
    return int64_t(std::stoi(line));
}

// Second line.
WorkingHours parse_working_hours(const std::string& line) {
    auto split_vec = split_str(line, " ");
    if (split_vec.size() != 2) {
        print_exit(line);
    }

    int64_t start_time = parse_time(split_vec[0]);
    int64_t end_time = parse_time(split_vec[1]);

    if (start_time == -1 || end_time == -1) {
        print_exit(line);
    }

    return WorkingHours{start_time, end_time};
}

// Event.
Event parse_event(const std::string& line) {
    auto split_vec = split_str(line, " ");

    int64_t time = parse_time(split_vec[0]);

    int64_t id = -1;
    if (is_positive_int(split_vec[1])) {
        id = std::stoi(split_vec[1]);
    }

    if (time == -1 || id == -1) {
        print_exit(line);
    }

    auto body =
        std::vector<std::string>(split_vec.begin() + 2, split_vec.end());

    return Event{time, id, body};
}