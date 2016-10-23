CXX = g++
CXXFLAGS = -Wall -std=c++11
target = bidding_system bidding_system_EDF customer

all: $(target)

bidding_system: src/bidding_system.cpp
	$(CXX) $(CXXFLAGS) src/bidding_system.cpp -o bidding_system

customer: src/customer.cpp
	$(CXX) $(CXXFLAGS) src/customer.cpp -o customer

bidding_system_EDF: src/bidding_system_EDF.cpp
	$(CXX) $(CXXFLAGS) src/bidding_system_EDF.cpp -o bidding_system_EDF

test_bidding_system: bidding_system customer
	./bidding_system test_data_customer

test_bidding_system_EDF: bidding_system_EDF
	./bidding_system_EDF test_data_EDF

clean:
	rm $(target)

