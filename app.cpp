#include "includes/project.hpp"

int main(int argc, char** argv){
    AUTO app(argv[1]);


    // AUTO * app;
    // while(true) {
    //     try {
    //         app = new AUTO(argv[1]);
    //     }
    //     catch(...) {
    //         Util::log("Critical error ocurred - restarting the system\n\n", true);
    //         delete app;
    //     }
    // }
}

// compiling command: g++ app.cpp includes/implementation/*.cpp -o AUTO -std=c++11 -lboost_system -lpthread -lcurlpp -lcurl 