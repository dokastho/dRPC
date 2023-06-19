# list of test drivers (with main()) for development
TESTSOURCES = $(wildcard test*.cpp)
# names of test executables
TESTS       = $(TESTSOURCES:%.cpp=%)

# list of sources used in project
SOURCES 	= $(wildcard *.cpp)
SOURCES     := $(filter-out $(TESTSOURCES), $(SOURCES))
# list of objects used in project
OBJECTS		= $(SOURCES:%.cpp=%.o)

SO_PATH = $(LD_LIBRARY_PATH)

LIB = drpc.so

#Default Flags
CXXFLAGS = -std=c++14 -Wconversion -Wall -Werror -Wextra -pedantic -pthread

# make debug - will compile "all" with $(CXXFLAGS) and the -g flag
#              also defines DEBUG so that "#ifdef DEBUG /*...*/ #endif" works
debug: CXXFLAGS += -g3 -DDEBUG
debug: clean all

py-package:
	sudo python3 setup.py install

# highest target; sews together all objects into executable
all: $(LIB) test_server test_kill test_basic test_many test_concurrent test_unreliable test_performance test_deaf

final: clean $(LIB)
	ln -f $(LIB) $(SO_PATH)

$(LIB): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(LIB) -shared

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(TESTS) $(PARTIAL_SUBMITFILE) $(FULL_SUBMITFILE)

# test1: test1.cpp $(LIB)
# 	$(CXX) $(CXXFLAGS) -o $@ $^
test_server: test_server.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^
test_kill: test_kill.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^
test_basic: test_basic.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^
test_many: test_many.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^
test_concurrent: test_concurrent.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^
test_unreliable: test_unreliable.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^
test_performance: test_performance.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^
test_deaf: test_deaf.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

# rule for creating objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -g -c $*.cpp
