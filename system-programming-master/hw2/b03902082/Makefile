CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
targets = bidding_system host player player_bonus

all: $(targets)

bidding_system: src/bidding_system.cpp
	$(CXX) $(CXXFLAGS) -o bidding_system src/bidding_system.cpp

host: src/host.cpp
	$(CXX) $(CXXFLAGS) -o host src/host.cpp

player: src/player.cpp
	$(CXX) $(CXXFLAGS) -o player src/player.cpp

player_bonus: src/player_bonus.cpp
	$(CXX) $(CXXFLAGS) -o player_bonus src/player_bonus.cpp

clean:
	rm -rf $(targets)
