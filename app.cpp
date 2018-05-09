#include "includes/project.hpp"

int main(int argc, char** argv){
    AUTO app = AUTO(argv[1]);
}

// compiling command: g++ app.cpp includes/implementation/*.cpp -o AUTO -std=c++11 -lboost_system -lpthread -lcurlpp -lcurlgit 