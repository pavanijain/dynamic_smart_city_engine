CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude

test_incident: tests/test_incident.cpp include/incident.hpp
	$(CXX) $(CXXFLAGS) tests/test_incident.cpp -o test_incident

clean:
	rm -f test_incident
