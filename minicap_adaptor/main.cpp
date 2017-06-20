#include <iostream>
#include <chrono>
#include <numeric>
#include <array>
#include <algorithm>
#include <thread>
#include <queue>
#include <atomic>
#include <mutex>
#include <libgen.h>
#include <stdio.h>
#include "minicap_adaptor.h"

using std::cout;
using std::cerr;
using std::endl;
using std::cin;
using std::array;
using std::thread;
using std::queue;
using std::chrono::duration;
using std::chrono::system_clock;
//using std::chrono::high_resolution_clock;

//static std::atomic_bool start(false);
static std::mutex buffer_mutex;
static char *new_buf = new char[buffer_size];
static char *last_buf = new char[buffer_size];
static uint32_t last_buf_len = 0;

void
outputer(const double framerate) {
    /*
    while (!start)
        std::this_thread::sleep_for(std::chrono::seconds(check_interval));
        */
    const auto begin = system_clock::now();
    const double timestep = 1.0 / framerate;
    uint32_t pic_id = 0;
    while (1) {
        duration<const double> diff = system_clock::now() - begin;
        if(diff.count() < pic_id * timestep) {
            std::this_thread::sleep_for(std::chrono::milliseconds(check_interval));
            continue;
        }
        //time to output next pic
        pic_id++;
        buffer_mutex.lock();
        fwrite(last_buf, 1, last_buf_len, stdout);
        //fflush(stdout);
        buffer_mutex.unlock();
    }
}

bool
read_pic(auto &pic_len) {
    static bool init = false;

    if(!init) {
        if(header_len != fread(new_buf, 1, header_len, stdin)) {
            cerr << "Broken header" << endl;
            return false;
        }
        init = true;
    }

    //todo while
    if (length_len != fread(&pic_len, 1, length_len, stdin) ) {
        cerr << "Broken length" << endl;
        return false;
    }
    //todo while
    if (pic_len != fread(new_buf, 1, pic_len, stdin) ) {
        cerr << "Broken pic" << endl;
        return false;
    }
    return true;
}

extern bool read_pic_un(uint32_t &pic_len, char * const new_buf);

int main(int argc, char *argv[]) {
    if(argc != 2) {
        cerr<<"Usage: "<<basename(argv[0])<<" framerate"<<endl;
        return -1;
    }
    thread t_output;
    bool start = false;
    uint32_t pic_len = 0;
    const double framerate = std::atof(argv[1]);

    while (1) {
        /*
        if(!read_pic(pic_len))
            break;
        */
        //todo option
        if(!read_pic_un(pic_len, new_buf))
            break;

        buffer_mutex.lock();
        std::swap(new_buf, last_buf);
        last_buf_len = pic_len;
        buffer_mutex.unlock();
        if(!start) {
            start = true;
            t_output = thread(outputer, framerate);
        }
    }
    //t_output.join();
    return 0;
}