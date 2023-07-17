# Executable and libraries

add_executable(task main.cpp computer_club.cpp line_parser.cpp)
target_include_directories(task PUBLIC ${CMAKE_SOURCE_DIR}/include)
