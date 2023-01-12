#pragma once
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <numeric>
#include <ios>
#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>

class ProgressTimer {
public:
    struct Stats
    {
        int n_done;
        int n_total;

        // estimated time of arrival in seconds
        double eta_s;

        // always in hz, but can divide by 'rate_div' to put in terms of
        // recommended 'rate_unit' (ie. KHz, MHz)
        double avg_rate_hz;
        std::string rate_unit;
        double rate_div;
    };

private:
    // time, iteration counters and deques for rate calculations
    std::chrono::time_point<std::chrono::system_clock> t_first = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> t_old = std::chrono::system_clock::now();
    std::vector<double> deq_t;
    std::vector<int> deq_n;
    int nupdates = 0;
    int period = 1;
    unsigned int smoothing = 50;
    bool use_ema = true;
    float alpha_ema = 0.1;

    Stats stats_; 

public:
    ProgressTimer() {
        // prealloc buffers
        stats_.rate_unit.reserve(10);
        deq_t.reserve(smoothing);
        deq_n.reserve(smoothing);

        reset();
    }

    const Stats &
    stats() const {
        return stats_;
    }

    void reset() {
        t_first = std::chrono::system_clock::now();
        t_old = std::chrono::system_clock::now();
        stats_.n_done = 0;
        deq_t.clear();
        deq_n.clear();
        period = 1;
        nupdates = 0;
        stats_.n_done = 0;
        stats_.n_total = 0;
        stats_.eta_s = 0.0;
        stats_.avg_rate_hz = 0.0;
        stats_.rate_div = 1.0;
        stats_.rate_unit = "Hz";
    }

    void finish() {
        progress(stats_.n_total,stats_.n_total);
    }

    void progress(int curr, int tot) {
        if(curr%period == 0) {
            stats_.n_total = tot;
            nupdates++;
            auto now = std::chrono::system_clock::now();
            double dt = ((std::chrono::duration<double>)(now - t_old)).count();
            double dt_tot = ((std::chrono::duration<double>)(now - t_first)).count();
            int dn = curr - stats_.n_done;
            stats_.n_done = curr;
            t_old = now;
            if (deq_n.size() >= smoothing) deq_n.erase(deq_n.begin());
            if (deq_t.size() >= smoothing) deq_t.erase(deq_t.begin());
            deq_t.push_back(dt);
            deq_n.push_back(dn);

            stats_.avg_rate_hz = 0.0;
            if (use_ema) {
                stats_.avg_rate_hz = deq_n[0] / deq_t[0];
                for (unsigned int i = 1; i < deq_t.size(); i++) {
                    double r = 1.0*deq_n[i]/deq_t[i];
                    stats_.avg_rate_hz = alpha_ema*r + (1.0-alpha_ema)*stats_.avg_rate_hz;
                }
            } else {
                double dtsum = std::accumulate(deq_t.begin(),deq_t.end(),0.);
                int dnsum = std::accumulate(deq_n.begin(),deq_n.end(),0.);
                stats_.avg_rate_hz = dnsum/dtsum;
            }

            // learn an appropriate period length to avoid spamming stdout
            // and slowing down the loop, shoot for ~25Hz and smooth over 3 seconds
            if (nupdates > 10) {
                period = (int)( std::min(std::max((1.0/25)*curr/dt_tot,1.0), 5e5));
                smoothing = 25*3;
            }
            stats_.eta_s = (tot-curr)/stats_.avg_rate_hz;
            double pct = (double)curr/(tot*0.01);
            if( ( tot - curr ) <= period ) {
                pct = 100.0;
                stats_.avg_rate_hz = tot/dt_tot;
                curr = tot;
                stats_.eta_s = 0;
            }

            stats_.rate_unit = "Hz";
            stats_.rate_div = 1.0;
            if (stats_.avg_rate_hz > 1e6) {
                stats_.rate_unit = "MHz"; stats_.rate_div = 1.0e6;
            } else if (stats_.avg_rate_hz > 1e3) {
                stats_.rate_unit = "kHz"; stats_.rate_div = 1.0e3;
            }
        }
    }
};
