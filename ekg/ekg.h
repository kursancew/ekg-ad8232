#ifndef _EKG_H_
#define _EKG_H_
#include "win_funcs.h"

#if !(defined(ARDUINO_PLATFORM) || defined(CORE_TEENSY) || defined(ARDUINO) || defined(__AVR__))
#include <chrono>
using namespace std;
unsigned long micros() {
    static auto start = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start).count();
}
#endif

class Cal {
    int wmin, wmax;
    int wlargest_d;
    int sum;
    int cnt;
    int cnt_max;
    int pmin, pmax, pavg;
    int prev_v;
    int largest_d;
    int badcnt;
    bool is_good;
    void reset_counters() {
        cnt = 0;
        badcnt = 0;
        sum = 0;
        wmin = 20000;
        wmax = 0;
        wlargest_d = 0;
    }
    void bad_sample() {
        ++badcnt;
        if (badcnt > 20) {
            reset_counters();
            is_good = false;
        }
    }
public:
    Cal(int cnt_max_) {
        pavg = -1;
        prev_v = 0;
        is_good = false;
        cnt_max = cnt_max_;
        reset_counters();
    }

    void print() {
        //printf("avg=%d max=%d min=%d maxd=%d\n", pavg, pmax, pmin, largest_d);
    }

    void set_cnt_max(int cnt_max_) { cnt_max = cnt_max_; }

    void swap_() {
        pavg = sum/cnt;
        pmax = wmax;
        pmin = wmin;
        largest_d = wlargest_d;
        reset_counters();
    }

    void push(int v, bool good) {
        if (!good) {
            bad_sample();
            return;
        }
        wlargest_d = max(abs(v-prev_v), wlargest_d);
        wmin = min(wmin, v);
        wmax = max(wmax, v);
        sum += v;
        ++cnt;
        if (cnt >= cnt_max) {
            is_good = true;
            swap_();
        }
        prev_v = v;
    }
    bool good() const {return is_good;}
    int get_avg() const {return pavg;}
    int get_min() const {return pmin;}
    int get_max() const {return pmax;}
    int get_maxd() const {return largest_d;}
};


class BeatFind {
    Cal &cal;
    int since_last;
    int last_v;
    int period_ms;
    constexpr static int WLEN = 4;
    AvgWindow<unsigned long, WLEN> d_bps;
    int beatcnt;
    unsigned long last_beat_time;
    static constexpr unsigned long min_beat_period_us = 150000;
public:
    BeatFind(Cal& c) : cal(c) {
        reset();
    }
    void reset() {
        last_v = 0;
        beatcnt = 0;
        last_beat_time = 0;
    }
    int push(int v) {
        int d = abs(v - last_v);
        int th = cal.get_maxd()/2;
        unsigned long since_last = micros() - last_beat_time;
        last_v = v;

        if (d > th && // delta greater than threshold
            since_last > min_beat_period_us) { // last detection was sufficient time ago
            unsigned long beat_time = micros();
            if(beatcnt) {
                d_bps.push(beat_time - last_beat_time);
            }
            last_beat_time = beat_time;
            ++beatcnt;
            return 1;
        }
        return 0;
    }
    unsigned int bpm() {
        if(beatcnt > WLEN) {
            unsigned long r = 60000000/d_bps.avg();
            return r;
        }
        return 0;
    }
};

constexpr static int taps[8] = {1,2,3,4,4,3,2,1};

class EKG {
    Cal cal;
    BeatFind beats;
    ConvWindow<int,8,4> cwin;
    int last_v;
public:
    enum {
        CALIBRATING,
        BEAT,
        NORM,
        BADSAMP,
    };

    EKG() : cal(300), beats(cal), cwin(taps) {}
    int push(int v, bool good) {
        v = good?cwin.push(v):v;
        cal.push(v, good);
        if(!cal.good()) return CALIBRATING;
        if(!good) return BADSAMP;
        last_v = v;
        if(beats.push(v)) {
            return BEAT;
        }
        return NORM;
    }
    unsigned int bpm() {
        return beats.bpm();
    }
    int v() { return last_v; }
    const Cal& get_cal() { return cal; }
};

#endif // _EKG_H_
