//
// Created by saleh on 11/30/24.
//

#include "timers.h"
#include <algorithm>
#include <cstdlib>

std::string timer_stats::legalize_filename(const std::string& name) const {
    std::string result = name;
    std::replace(result.begin(), result.end(), ' ', '_');
    std::replace(result.begin(), result.end(), '/', '_');
    std::replace(result.begin(), result.end(), '\\', '_');
    return result;
}

std::string timer_stats::pairs_to_string() const {
    std::string result = ".";
    for (auto &pair : pairs) {
        result += legalize_filename(pair.first) + "_" + std::to_string(pair.second) + ".";
    }
    if (pairs.size() > 0) {
        result.pop_back();
    }
    return result;
}

std::string timer_stats::pairs_to_json() const {
    std::string result = "{";
    for (auto &pair : pairs) {
        result += "\"" + pair.first + "\": " + std::to_string(pair.second) + ", ";
    }
    // remove the last comma and space if its not empty
    if (pairs.size() > 0) {
        result.pop_back();
        result.pop_back();
    }
    result += "}";
    return result;
}

std::string timer_stats::data_to_json() const {
    std::string result = "[";
    for (auto &s : samples) {
        result += std::to_string(s) + ", ";
    }
    // remove the last comma and space if its not empty
    if (pairs.size() > 0) {
        result.pop_back();
        result.pop_back();
    }
    result += "]";
    return result;
}

float timer_stats::ave() const {
    // Calculate the average with vector
    float sum = 0;
    for (auto t : samples) {
        sum += t;
    }
    return sum / samples.size();
}

float timer_stats::max() const {
    float max = samples[0];
    for (size_t i = 1; i < samples.size(); i++) {
        if (samples[i] > max) {
            max = samples[i];
        }
    }
    return max;
}

float timer_stats::min() const {
    float min = samples[0];
    for (size_t i = 1; i < samples.size(); i++) {
        if (samples[i] < min) {
            min = samples[i];
        }
    }
    return min;
}

float timer_stats::median() const {
    std::vector<float> v = samples;
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    if (n % 2 == 0) {
        return (v[n / 2 - 1] + v[n / 2]) / 2;
    } else {
        return v[n / 2];
    }
}

float timer_stats::variance() const {
    float avg = ave();
    float sum = 0;
    for (auto t : samples) {
        sum += (t - avg) * (t - avg);
    }
    return sum / samples.size();
}

void timer_stats::print() const {
    std::cout << "============================================" << std::endl;
    std::cout << "Stats for " << name << " with " << pairs_to_json() << " :" << std::endl;
    std::cout << ">>Median:  \t" << median() << std::endl;
    std::cout << "> Average: \t" << ave() << std::endl;
    std::cout << "> Samples: \t" << count() << std::endl;
    std::cout << "> Variance:\t" << variance() << std::endl;
    std::cout << "> Max:     \t" << max() << std::endl;
    std::cout << "> Min:     \t" << min() << std::endl;
    std::cout << "============================================" << std::endl;
}

void timer_stats::save() const {
    std::ofstream file;
    file.open("stats_" + legalize_filename(name) + pairs_to_string() + ".json", std::ios::app);
    // save it in json format
    file << "{\n";
    file << "\"name\": \"" << name << "\",\n";
    file << "\"pairs\": " << pairs_to_json() << ",\n";
    file << "\"samples\": " << count() << ",\n";
    file << "\"data\": " << data_to_json() << ", \n";
    file << "\"average\": " << ave() << ",\n";
    file << "\"median\": " << median() << ",\n";
    file << "\"variance\": " << variance() << ",\n";
    file << "\"max\": " << max() << ",\n";
    file << "\"min\": " << min() << "\n";
    file << "}\n";
    file.close();
}

timer_scope::~timer_scope() {
    if(m_bIsRoot) {
        report_from_last(name);
    } else {
        m_pStats->add_sample(from_last());
    }
}
