//
// Created by saleh on 9/30/24.
//

#pragma once

#include <atomic>
#include <chrono>
#include <fstream>
#include <thread>
#include <vector>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <malloc.h>
#include <map>
#include <sstream>

class mem_usage_tracker {
protected:
  struct DataPoint {
    double memoryUsageInGB;
    double totalAllocatedSpace;
    double totalInUseSpace;
    double totalFreeSpace;
    double externalFragmentation;
    char timeOfSample[24];
    DataPoint(double memUsage, double totalAllocated, double totalInUse,
              double totalFree, double extFrag, std::string time)
        : memoryUsageInGB(memUsage), totalAllocatedSpace(totalAllocated),
          totalInUseSpace(totalInUse), totalFreeSpace(totalFree),
          externalFragmentation(extFrag)

    {
      for (size_t i = 0; i < time.size(); i++) {
        timeOfSample[i] = time[i];
      }
    }
  };

  const size_t m_iWriteEvery = 3;
  const size_t m_iFlushEvery = 8;
  const int m_iInterval;
  const double m_dMemUsageLimit;
  const std::string m_strFilePath;
  std::atomic<bool> stopFlag;
  std::thread m_oThread;
  std::vector<DataPoint> m_vPendingWrite;
  std::ofstream m_oFile;
  size_t m_lCounter = 0;

public:
  mem_usage_tracker(int interval, double memUsageLimit,
                    const std::string &baseDirPath)
      : m_iInterval(interval), m_dMemUsageLimit(memUsageLimit),
        m_strFilePath(baseDirPath + "MemUsageTracker.txt") {
    stopFlag.store(false);
    m_lCounter = 0;
  }

  ~mem_usage_tracker() {
    requestStopAndWait();
  }

  std::string getDateTimeString() {
    // Get the current time
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

    // Convert to local time
    std::tm local_tm = *std::localtime(&now_time_t);

    // Create a stringstream to format the date and time with zero padding.
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y/%m/%d %H:%M:") << std::setw(2)
        << std::setfill('0') << local_tm.tm_sec << '.' << std::setw(3)
        << std::setfill('0') << now_ms.count();

    // Store the formatted date and time in a string variable
    std::string dateTimeStr = oss.str();
    return dateTimeStr;
  }

  void startInSeparateThread() {
    try {
      m_oFile = std::ofstream(m_strFilePath);
      m_oThread = std::thread(&mem_usage_tracker::trackMemoryUsage, this);
    } catch (std::exception &e) {
      std::cerr << "Error in StartInSeparateThread(): " << e.what() << std::endl;
    }
  }

  void requestStopAndWait() {
    stopFlag.store(true);
    m_oThread.join();
    m_oFile.flush();
    m_oFile.close();
    // std::cout << "The memory tracker thread has stopped." << std::endl;
    // std::cout << "The memory tracker file is saved at: " << m_strFilePath << std::endl;
  }

protected:
  void writeToFileIfNeeded(std::string timeOfSample, double memUsage,
                           std::map<std::string, double> dictDetailed) {
    m_vPendingWrite.push_back(
        {memUsage, dictDetailed["totalAllocatedSpace"],
         dictDetailed["totalInUseSpace"], dictDetailed["totalFreeSpace"],
         dictDetailed["externalFragmentation"], timeOfSample.data()});
    if (m_vPendingWrite.size() % m_iFlushEvery == 0) {
      m_oFile.flush();
    }

    if (m_vPendingWrite.size() >= m_iWriteEvery) {
      for (auto &dataPoint : m_vPendingWrite) {
        m_oFile << dataPoint.timeOfSample << "," << dataPoint.memoryUsageInGB
                << std::endl;
      }
      m_vPendingWrite.clear();
    }
  }

  // Function to be run in a separate thread
  void trackMemoryUsage() {
    while (!stopFlag.load()) {
      auto usageGigs = getMemoryUsage();
      auto dictDetailed = getMemoryUsageDetailed();
      if (m_lCounter++ % 5 == 0) {
        std::cerr << "######################  Current memory usage: " << std::fixed
                  << std::setprecision(2) << usageGigs << " GB, Allocated: "
                  << std::fixed << std::setprecision(2)
                  << dictDetailed["totalAllocatedSpace"] << " GB, In-use: "
                  << std::fixed << std::setprecision(2)
                  << dictDetailed["totalInUseSpace"] << " GB, Ext-fragmantation: "
                  << std::fixed << std::setprecision(2)
                  << dictDetailed["externalFragmentation"] << "  ****"
                  << std::endl;
      }
      if (usageGigs > m_dMemUsageLimit) {
        std::cerr << "Memory usage is too high! Exiting..." << std::endl;
        std::exit(99);
      }
      std::cerr.flush();
      writeToFileIfNeeded(getDateTimeString(), usageGigs, dictDetailed);
      std::this_thread::sleep_for(std::chrono::seconds(m_iInterval));
    }
    // std::cout << "Memory tracker thread is exiting..." << std::endl;
  }

protected:
  // Function to get the current memory usage in KB
  double getMemoryUsage() {
    std::ifstream procStatus("/proc/self/status");
    std::string line;
    while (std::getline(procStatus, line)) {
      if (line.substr(0, 6) == "VmSize") {
        long memoryUsageKB = std::stol(
            line.substr(line.find_last_of('\t'), line.find_last_of('k') - 1));
        return static_cast<double>(memoryUsageKB) /
               1048576.0; // Convert KB to GB
      }
    }
    return -1; // Return -1 if VmSize line is not found
  }

  std::map<std::string, double> getMemoryUsageDetailed() {
    std::map<std::string, double> memUsage;
    struct mallinfo2 mi = mallinfo2();
    memUsage["totalAllocatedSpace"] = (double) mi.arena / 1024.0 / 1024.0 / 1024.0;
    memUsage["totalInUseSpace"] = mi.uordblks/ 1024.0 / 1024.0 / 1024.0;
    memUsage["totalFreeSpace"] = mi.fordblks/ 1024.0 / 1024.0 / 1024.0;
    memUsage["largestAllocatableBlock"] = mi.hblkhd/ 1024.0 / 1024.0 / 1024.0;
    memUsage["externalFragmentation"] =
        1.0 - static_cast<double>(mi.hblkhd) / static_cast<double>(mi.fordblks);
    return memUsage;
  }
};
