# list of test drivers (with main()) for development
TESTSOURCES = $(wildcard test*.cpp)
# names of test executables
TESTS       = $(TESTSOURCES:%.cpp=%)

# list of sources used in project
SOURCES 	= $(wildcard *.cpp)
SOURCES     := $(filter-out $(TESTSOURCES), $(SOURCES))
# list of objects used in project
OBJECTS		= $(SOURCES:%.cpp=%.o)

LIB = drpc.so

#Default Flags
CXXFLAGS = -std=c++14 -Wconversion -Wall -Werror -Wextra -pedantic

# make debug - will compile "all" with $(CXXFLAGS) and the -g flag
#              also defines DEBUG so that "#ifdef DEBUG /*...*/ #endif" works
debug: CXXFLAGS += -g3 -DDEBUG
debug: clean all

# highest target; sews together all objects into executable
all: $(LIB)

$(LIB): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(LIB) -shared 

# Automatically generate any build rules for test*.cpp files
# define make_tests
#     ifeq ($$(PROJECTFILE),)
# 	    @echo Edit PROJECTFILE variable to .cpp file with main\(\)
# 	    @exit 1
#     endif
#     SRCS = $$(filter-out $$(PROJECTFILE), $$(SOURCES))
#     OBJS = $$(SRCS:%.cpp=%.o)
#     HDRS = $$(wildcard *.h)
#     $(1): CXXFLAGS += -g3 -DDEBUG
#     $(1): $$(OBJS) $$(HDRS) $(1).cpp
# 	$$(CXX) $$(CXXFLAGS) $$(OBJS) $(1).cpp -o $(1)
# endef
# $(foreach test, $(TESTS), $(eval $(call make_tests, $(test))))

alltests: clean $(TESTS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) $(TESTS) $(PARTIAL_SUBMITFILE) $(FULL_SUBMITFILE)

# rule for creating objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -g -c $*.cpp

# Compile the file server and tag this compilation
fs_server: ${FS_OBJS} libfs_server.o
	./autotag.sh
	${CC} -o $@ $^ -pthread -ldl
