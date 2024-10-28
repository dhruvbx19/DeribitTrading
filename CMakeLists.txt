cmake_minimum_required(VERSION 3.10)
project(DeribitTrading)

set(CMAKE_CXX_STANDARD 17)

# Find cURL
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
link_directories(${CURL_LIBRARY_DIRS})

# Find Boost
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
include_directories(include)  # Add this line if json.hpp is in the 'include' folder

# Add executable
add_executable(DeribitTrading main.cpp)

# Link libraries
target_link_libraries(DeribitTrading ${CURL_LIBRARIES} ${Boost_LIBRARIES})
