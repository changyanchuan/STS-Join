# yanchuan (2020-10)

CXX = g++ -std=c++11
CXXFLAGS = -O3

INCLUDES = -I./

APPS = test_cdds

all:
	$(foreach APP, $(APPS), $(CXX) $(CXXFLAGS) $(INCLUDES) $(APP).cpp -o $(APP);)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean  

clean:  
	rm -f ${APPS} && find . -name "*.o" -exec rm -rf {} \;
