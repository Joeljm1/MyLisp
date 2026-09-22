CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic -O0 -g

COMMON_SRCS = Environemnt.cpp printer.cpp reader.cpp string.cpp type.cpp

.PHONY: all clean

all: step0_repl step1_read_print step2_eval step3_env step4_if_fn_do

step0_repl: step0_repl.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

step1_read_print: step1_read_mycpp.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

step2_eval: step2_eval.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

step3_env: step3_env.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

step4_if_fn_do: step4_if_fn_do.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -lreadline -o $@

clean:
	rm -f step0_repl step1_read_print step2_eval step3_env step4_if_fn_do
