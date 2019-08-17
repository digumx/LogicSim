echo building all targets
echo building release
g++ --std=c++11 -Wall -Wno-unused-but-set-variable -I includes/ -lncurses -o binaries/logicsim sources/*.cpp includes/*.hpp includes/*.h
echo building debug
g++ -g --std=c++11 -Wall -Wno-unused-but-set-variable -I includes/ -lncurses -o binaries/logicsim.debug sources/*.cpp includes/*.hpp includes/*.h
