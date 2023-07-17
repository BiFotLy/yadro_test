#include <algorithm>
#include <computer_club.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <line_parser.hpp>
#include <string>
#include <vector>

void ComputerClub::parse_file(const std::string& filename) {
    std::string line;
    std::ifstream file;
    file.open(filename);

    if (!file.is_open()) {
        std::perror("Error");
        std::exit(EXIT_FAILURE);
    }

    // First line.
    std::getline(file, line);
    this->setup_tables(line);

    // Second line.
    std::getline(file, line);
    this->setup_working_hours(line);

    // Third line.
    std::getline(file, line);
    this->setup_hour_cost(line);

    // Parse and process all incoming events.
    while (std::getline(file, line)) {
        this->add_incoming_event(line);
        if (!this->process_event(this->incoming_events.back())) {
            print_exit(line);
        };
    }

    file.close();

    // There are clients at the end of the working hours.
    std::vector<std::string> clients_left;
    for (auto client : this->clients) {
        clients_left.push_back(client.first);
    }
    std::sort(clients_left.begin(),
              clients_left.end());  // Clients sorted by name.

    // All remaining clients must leave after club closes.
    for (std::string client_name : clients_left) {
        generate_client_leave(this->working_hours.end, client_name);
    }
}

void ComputerClub::setup_tables(const std::string& line) {
    int64_t number_of_tables = parse_positive_number(line);
    for (int idx = 1; idx <= number_of_tables; ++idx) {
        this->tables[idx] = Table{};
    }
    this->empty_tables = number_of_tables;
}

void ComputerClub::setup_working_hours(const std::string& line) {
    this->working_hours = parse_working_hours(line);
}

void ComputerClub::setup_hour_cost(const std::string& line) {
    this->hour_cost = parse_positive_number(line);
}

void ComputerClub::add_incoming_event(const std::string& line) {
    this->incoming_events.push_back(parse_event(line));
}

bool ComputerClub::process_event(const Event& event) {
    std::string client_name = event.body[0];
    if (!is_name_correct(client_name)) {
        return false;
    }
    int64_t start = this->working_hours.start, end = this->working_hours.end;
    switch (event.id) {
        // Client arrived.
        case 1: {
            // Client tried to enter too early or too soon.
            if ((event.time < start) || (event.time >= end)) {
                this->generate_outgoing_event(event.time, 13, "NotOpenYet");
                break;
            }

            // Client is already in club.
            if (this->clients.count(client_name) == 1) {
                this->generate_outgoing_event(event.time, 13,
                                              "YouShallNotPass");
                break;
            }

            // Add client to club.
            this->clients[client_name] = Client{-1, false};
            break;
        }

        // Client sat at table.
        case 2: {
            // Check whether table number exists and is a positive int.
            if ((event.body.size() != 2) || (!is_positive_int(event.body[1]))) {
                return false;
            }

            int64_t from_table = this->clients[client_name].table_number;
            int64_t to_table = std::stoi(event.body[1]);

            // Client wasn't in club.
            if (this->clients.count(client_name) == 0) {
                this->generate_outgoing_event(event.time, 13, "ClientUnknown");
                break;
            }

            // Table is already taken.
            if (this->tables[to_table].is_busy()) {
                this->generate_outgoing_event(event.time, 13, "PlaceIsBusy");
                break;
            }

            // Client doesn't have table yet.
            if (from_table == -1) {
                this->generate_client_sit_at_table(event.time, client_name,
                                                   to_table);
            } else {
                // Client have table and want to switch it.
                this->generate_client_switch_table(event.time, client_name,
                                                   from_table, to_table);
            }
            break;
        }

        // Client waits in queue.
        case 3: {
            // There is empty tables.
            if (this->empty_tables != 0) {
                this->generate_outgoing_event(event.time, 13,
                                              "ICanWaitNoLonger!");
                break;
            }

            // Queue size is bigger than number of tables.
            if (this->clients_queue.size() > this->tables.size()) {
                this->generate_client_leave(event.time, client_name);
                break;
            }

            // If client isn't in queue, then put him in.
            this->add_to_queue(client_name);
            break;
        }

        // Client left.
        case 4: {
            // Client isn't in club.
            if (this->clients.count(client_name) == 0) {
                this->generate_outgoing_event(event.time, 13, "ClientUnknown");
                break;
            }

            // Client's table.
            int64_t table_number = this->clients[client_name].table_number;

            // Remove from queue, if waiting.
            this->remove_from_queue(client_name);

            // Remove from list of clients.
            this->client_leave(event.time, client_name);

            // Give empty table to client from queue.
            this->generate_queue_client_gets_table(event.time, table_number);
            break;
        }
    }
    return true;
}

void ComputerClub::generate_outgoing_event(int64_t time, int64_t id,
                                           std::string body_text) {
    this->outgoing_events.push_back(Event{time, id, split_str(body_text, " ")});
}

void ComputerClub::client_leave(int64_t time, const std::string& client_name) {
    // Mark session as ended where client sat.
    int64_t client_table_number = this->clients[client_name].table_number;
    if (client_table_number != -1) {
        this->tables[client_table_number].end_session(time);
    }

    // Delete from list of clients.
    this->clients.erase(client_name);
}

void ComputerClub::generate_client_leave(int64_t time,
                                         const std::string& client_name) {
    this->generate_outgoing_event(time, 11, client_name);

    this->client_leave(time, client_name);
}

void ComputerClub::generate_client_sit_at_table(int64_t time,
                                                const std::string& client_name,
                                                int64_t to_table) {
    this->clients[client_name].table_number = to_table;
    this->tables[to_table].start_session(time, client_name);
    this->empty_tables -= 1;
}

void ComputerClub::generate_client_switch_table(int64_t time,
                                                const std::string& client_name,
                                                int64_t from_table,
                                                int64_t to_table) {
    // End the session at previous table.
    this->tables[from_table].end_session(time);

    this->generate_client_sit_at_table(time, client_name, to_table);

    // Give freed table to client from queue.
    this->generate_queue_client_gets_table(time, to_table);
}

void ComputerClub::generate_queue_client_gets_table(int64_t time,
                                                    int64_t to_table) {
    // If client queue is not empty.
    if (!this->clients_queue.empty()) {
        std::string first_client_in_queue = this->clients_queue.front();

        this->clients[first_client_in_queue].table_number = to_table;
        this->clients[first_client_in_queue].is_waiting = false;

        this->tables[to_table].start_session(time, first_client_in_queue);
        this->generate_outgoing_event(
            time, 12, first_client_in_queue + " " + std::to_string(to_table));

        this->clients_queue.pop_front();
    }
}

void ComputerClub::add_to_queue(const std::string& client_name) {
    if (!this->clients[client_name].is_waiting) {
        this->clients[client_name].is_waiting = true;
        this->clients_queue.push_back(client_name);
    }
}

void ComputerClub::remove_from_queue(const std::string& client_name) {
    if (this->clients[client_name].is_waiting) {
        this->clients_queue.erase(std::remove_if(
            this->clients_queue.begin(), this->clients_queue.end(),
            [client_name](std::string queue_client_name) {
                return queue_client_name == client_name;
            }));
    }
}

std::string ComputerClub::get_fmt_results() {
    std::string results;

    results += get_time_str(this->working_hours.start) + "\n";

    // Merge vectors with incoming and outgoing events into one.
    std::vector<Event> all_events;
    std::copy(this->incoming_events.begin(), this->incoming_events.end(),
              std::back_inserter(all_events));
    std::copy(this->outgoing_events.begin(), this->outgoing_events.end(),
              std::back_inserter(all_events));

    // Sort the resulting vector by time.
    // Incoming events will always come before outgoing.
    std::sort(all_events.begin(), all_events.end(),
              [](Event e1, Event e2) { return e1.time < e2.time; });

    // Format all events.
    for (auto const& event : all_events) {
        std::string body;
        for (const auto& str : event.body) {
            body += " " + str;
        }
        results += get_time_str(event.time) + " " + std::to_string(event.id) +
                   body + "\n";
    }

    results += get_time_str(this->working_hours.end) + "\n";

    // Format resulting revenue and total time in hh:mm.
    for (auto table : this->tables) {
        int64_t total_earnings =
            table.second.get_total_paid_hours() * this->hour_cost;
        int64_t total_time_in_mins = table.second.get_total_time();
        results += std::to_string(table.first) + " " +
                   std::to_string(total_earnings) + " " +
                   get_time_str(total_time_in_mins) + "\n";
    }

    return results;
}

void Table::start_session(int64_t time, std::string client_name) {
    this->sessions.push_back(ClientSession{time, -1, client_name});
}

void Table::end_session(int64_t time) { this->sessions.back().end = time; }

bool Table::is_busy() {
    return (!this->sessions.empty()) && (this->sessions.back().end == -1);
}

int64_t Table::get_total_time() {
    int64_t total_time = 0;
    for (auto session : this->sessions) {
        total_time += session.end - session.start;
    }
    return total_time;
}

int64_t Table::get_total_paid_hours() {
    int64_t total_paid_hours = 0;
    for (auto session : this->sessions) {
        total_paid_hours += (session.end - session.start - 1) / 60 + 1;
    }
    return total_paid_hours;
}