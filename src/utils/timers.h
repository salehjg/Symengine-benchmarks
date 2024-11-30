//
// Created by saleh on 11/5/24.
//

#pragma once

#include <iostream>
#include <chrono>
#include <functional>
#include <string>
#include <memory>
#include <fstream>
#include <map>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

class timer_scope; // Forward declaration
class timer_stats {
    friend class timer_scope;

private:
    const std::string name;
    std::vector<float> samples;
    const std::map<std::string, int> pairs;

    std::string legalize_filename(const std::string& name) const;

    std::string pairs_to_string() const;

    std::string pairs_to_json() const;

    std::string data_to_json() const;

public:
    timer_stats(const std::string& name) : name(name) {
    }

    timer_stats(const std::string& name, const std::map<std::string, int>& pairs) : name(name), pairs(pairs) {
    }

    void add_sample(float time) {
        samples.push_back(time);
    }

    size_t count() const {
        return samples.size();
    }

    float ave() const;

    float max() const;

    float min() const;

    float median() const;

    float variance() const;

    void print() const;

    void save() const;

    ~timer_stats() {
        print();
        save();
    }
};

class timer_scope {
private:
    std::chrono::system_clock::time_point m_oTimerLast;
    const std::string name;
    const bool m_bIsRoot;
    timer_stats* m_pStats; // to keep things simple, we are not using smart pointers.

public:
    timer_scope(const std::string& name) : name(name), m_bIsRoot(true) {
        m_oTimerLast = high_resolution_clock::now();
        m_pStats = nullptr;
    }

    timer_scope(timer_stats& parent) : name(""), m_bIsRoot(false) {
        m_oTimerLast = high_resolution_clock::now();
        m_pStats = &parent;
    }

    ~timer_scope();

    template <class StdTimeResolution = std::milli>
    float from_last() {
        auto now = high_resolution_clock::now();
        duration<float, StdTimeResolution> ms = now - m_oTimerLast;
        m_oTimerLast = now;
        return ms.count();
    }

    template <class StdTimeResolution = std::milli>
    float report_from_last(const std::string& msg = "") {
        auto t = from_last<StdTimeResolution>();
        std::cout << "Elapsed " << msg << ": " << t << " ." << std::endl;
        return t;
    }

    template <class StdTimeResolution = std::milli>
    static inline float for_lambda(const std::function<void()>& operation) {
        auto t1 = high_resolution_clock::now();
        operation();
        auto t2 = high_resolution_clock::now();
        duration<float, StdTimeResolution> ms = t2 - t1;
        return ms.count();
    }

    template <class StdTimeResolution = std::milli>
    static inline float report_for_lambda(const std::function<void()>& operation) {
        auto t = for_lambda<StdTimeResolution>(operation);
        std::cout << "Elapsed: " << t << " ." << std::endl;
        return t;
    }
};
