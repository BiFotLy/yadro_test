#ifndef COMPUTER_CLUB_H_
#define COMPUTER_CLUB_H_

#include <cstdint>
#include <map>
#include <queue>
#include <string>
#include <vector>

struct ClientSession {
    int64_t start;
    int64_t end;
    std::string client_name;
};

struct WorkingHours {
    int64_t start;
    int64_t end;
};

class Table {
   public:
    void start_session(int64_t time, std::string client_name);
    void end_session(int64_t time);
    int64_t get_total_time();
    int64_t get_total_paid_hours();
    bool is_busy();

   private:
    std::vector<ClientSession> sessions;
};

struct Client {
    int64_t table_number;
    bool is_waiting;
};

struct Event {
    int64_t time;
    int64_t id;
    std::vector<std::string> body;
};

class ComputerClub {
   public:
    void parse_file(const std::string& filename);
    std::string get_fmt_results();

   private:
    void setup_tables(const std::string& line);
    void setup_working_hours(const std::string& line);
    void setup_hour_cost(const std::string& line);

    void add_incoming_event(const std::string& line);
    bool process_event(const Event& event);
    void generate_outgoing_event(int64_t time, int64_t id,
                                 std::string body_text);

    void client_leave(int64_t time, const std::string& client_name);
    void generate_client_leave(int64_t time, const std::string& client_name);

    void generate_client_sit_at_table(int64_t time,
                                      const std::string& client_name,
                                      int64_t to_table);
    void generate_client_switch_table(int64_t time,
                                      const std::string& client_name,
                                      int64_t from_table, int64_t to_table);

    void generate_queue_client_gets_table(int64_t time, int64_t to_table);
    void add_to_queue(const std::string& client_name);
    void remove_from_queue(const std::string& client_name);

    WorkingHours working_hours;
    int64_t hour_cost;

    std::map<int64_t, Table> tables;
    int64_t empty_tables;
    std::map<std::string, Client> clients;
    std::deque<std::string> clients_queue;

    std::vector<Event> incoming_events;
    std::vector<Event> outgoing_events;
};

#endif