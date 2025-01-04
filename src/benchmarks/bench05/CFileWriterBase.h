//
// Created by saleh on 12/29/24.
//

// This is a base class for CFileWriterX classes.

#pragma once

#include "symengine/basic.h"
#include "symengine/serialize-cereal.h"
#include <iostream>

#include <fstream>
#include <vector>
#include <stdexcept>
#include <iomanip>

#include "json/json.h"
#include <boost/filesystem.hpp>


/**
 * To use this class, SymEngine must be built using the external Cereal library. Otherwise `"symengine/serialize-cereal.h"` won't be available.
 * A class to provide READ/WRITE operations to a file through SymEngine's streaming serializer.
 * The scheme used:
 *  - Generate a ReturnID (RetID).
 *  - Write elements to the RetID.
 *  - Read elements from the RetID.
 * So, each RetID is a collection of elements. The catch is that these retIDs can be interleaved for read and write.
 *
 * The class create a file if it does not exist, and starts in the read/write mode.
 * If the file exists, the class starts in read-only mode. (for now)
 *
 * This base class provides API to:
 *  - Create RetID
 *  - Write one or more elements to each RetID
 *  - Track the information of each RetID's elements.
 *  - Variadic read/write functions for different types of elements.
 * This class manages stream to allow interleaved read/write operations to any RetID.
 * For example, this could be an acceptable use case:
 *  - Write 1 element to RetID 0
 *  - Write 1 element to RetID 0
 *  - Write 1 element to RetID 1
 *  - Write 2 element to RetID 1
 *  - Write 1 element to RetID 0
 *  - Read 1 element from RetID 1, element offset 0.
 *  - Read 1 element from RetID 0, element offset 1.
 *
 * We only have 1 file, so all the offsets for (retID, elementIndex) should be tracked and stored.
 */
template<typename... Types>
class CFileWriterBase {
protected:
    const std::string m_sFormat = "RawFmt01";
    const std::string m_sName, m_sBasePath, m_sFileBin, m_sFileJson;
    const bool m_bDebug;

    std::mutex m_oMutexOffsets;
    std::unordered_map<size_t, std::vector<std::streampos> > m_mOffsets;

    std::streampos m_lOffset = 0;
    std::mutex m_oMutexRetId;
    size_t m_lRetId = 0;
    bool m_bNuked = false;

    Json::Value m_oFileJson;
    std::fstream m_oFileBin;

    std::unique_ptr<SymEngine::RCPBasicAwareOutputArchive<cereal::PortableBinaryOutputArchive> > m_oArchiveSave;
    std::unique_ptr<SymEngine::RCPBasicAwareInputArchive<cereal::PortableBinaryInputArchive> > m_oArchiveLoad;

    class FileError : public std::runtime_error {
    public:
        FileError(const std::string &msg) : std::runtime_error(msg) {
        }
    };

public:
    CFileWriterBase(
        const std::string &basePath,
        const std::string &name,
        bool load_if_exists,
        bool dbg = false
    ) try : m_sBasePath(basePath),
            m_sFileBin(basePath + name + ".bin"),
            m_sFileJson(basePath + name + ".json"),
            m_sName(name),
            m_bDebug(dbg) {
        try {
            bool binExists = false;
            bool jsonExists = false;

            try {
                std::fstream binFile(m_sFileBin);
                binExists = binFile.good();
                std::ifstream jsonFile(m_sFileJson);
                jsonExists = jsonFile.good();
            } catch (const std::exception &e) {
                throw FileError("Failed to check file existence: " + std::string(e.what()));
            }

            if (binExists && jsonExists && load_if_exists) {
                try {
                    LoadExistingFiles();
                } catch (const std::exception &e) {
                    throw FileError("Failed to load existing files: " + std::string(e.what()));
                }
            } else {
                try {
                    CreateNewFiles();
                } catch (const std::exception &e) {
                    throw FileError("Failed to create new files: " + std::string(e.what()));
                }
            }

            InitializeArchives();
        } catch (const std::exception &e) {
            Cleanup();
            throw;
        }
    } catch (...) {
        throw;
    }

    ~CFileWriterBase() {
        try {
            if (!m_bNuked) {
                SaveBookkeepingData();
            } else {
                debugPrint("This instance is nuked. No need to write the json file.");
            }
        } catch (const std::exception &e) {
            debugPrint("Error in destructor: ", e.what());
        }
    }

    void Append(size_t retId, const std::tuple<Types...> &data) {
        try {
            std::lock_guard<std::mutex> lock(m_oMutexOffsets);
            std::apply([&](const Types &... args) {
                _Append(retId, args...);
            }, data);
        } catch (const std::exception &e) {
            throw FileError("Failed to append data: " + std::string(e.what()));
        }
    }

    std::tuple<Types...> Read(size_t retId, size_t stateIndex) {
        try {
            std::lock_guard<std::mutex> lock(m_oMutexOffsets);
            if (m_mOffsets.find(retId) == m_mOffsets.end()) {
                throw FileError("Invalid retId: " + std::to_string(retId));
            }
            if (stateIndex >= m_mOffsets[retId].size()) {
                throw FileError("Invalid state index: " + std::to_string(stateIndex));
            }

            std::tuple<Types...> data;
            std::apply([&](Types &... args) {
                std::streampos addr = m_mOffsets[retId][stateIndex];
                m_oFileBin.seekg(addr);
                if (!m_oFileBin.good()) {
                    throw FileError("Failed to seek to position in binary file");
                }
                (*m_oArchiveLoad)(args...);
            }, data);
            return data;
        } catch (const std::exception &e) {
            throw FileError("Failed to read data: " + std::string(e.what()));
        }
    }

    size_t GetElementCount(size_t retId) {
        try {
            std::lock_guard<std::mutex> lock(m_oMutexOffsets);
            return m_mOffsets[retId].size();
        } catch (const std::exception &e) {
            throw FileError("Failed to get element count: " + std::string(e.what()));
        }
    }

    void Nuke() {
        try {
            std::lock_guard<std::mutex> lock(m_oMutexOffsets);
            debugPrint("CFileWriter: Nuking the instance...");
            m_bNuked = true;
            m_oFileBin.close();

            if (!DeleteFileIfExists(m_sFileBin) || !DeleteFileIfExists(m_sFileJson)) {
                throw FileError("Failed to delete one or more files during nuke operation");
            }

            m_mOffsets.clear();
            m_oFileJson.clear();
        } catch (const std::exception &e) {
            throw FileError("Failed to nuke instance: " + std::string(e.what()));
        }
    }

    size_t PeekRetId() {
        try {
            std::lock_guard<std::mutex> lock(m_oMutexRetId);
            if (m_lRetId == 0) {
                throw FileError("No retId has been generated yet");
            }
            return m_lRetId - 1;
        } catch (const std::exception &e) {
            throw FileError("Failed to peek retId: " + std::string(e.what()));
        }
    }

    size_t GenerateRetId() {
        try {
            std::lock_guard<std::mutex> lock(m_oMutexRetId);
            return m_lRetId++;
        } catch (const std::exception &e) {
            throw FileError("Failed to generate retId: " + std::string(e.what()));
        }
    }

protected:
    void LoadExistingFiles() {
        std::ifstream jsonFile(m_sFileJson);
        if (!jsonFile.is_open()) {
            throw FileError("Failed to open JSON file for reading");
        }

        try {
            jsonFile >> m_oFileJson;
            DeserializeOffsets(m_oFileJson["offsets"]);

            if (m_oFileJson["meta"]["name"].asString() != m_sName ||
                m_oFileJson["meta"]["format"].asString() != m_sFormat) {
                throw FileError("JSON file configuration mismatch");
            }

            m_lOffset = m_oFileJson["meta"]["fileOffset"].asUInt64();
            m_lRetId = m_oFileJson["meta"]["retId"].asUInt64();

            m_oFileBin.open(m_sFileBin, std::ios::in | std::ios::out | std::ios::binary);
            if (!m_oFileBin.is_open()) {
                throw FileError("Failed to open binary file");
            }

            debugPrint("Loaded files successfully");
        } catch (const Json::Exception &e) {
            throw FileError("JSON parsing error: " + std::string(e.what()));
        }
    }

    void CreateNewFiles() {
        m_oFileJson["meta"]["name"] = m_sName;
        m_oFileJson["meta"]["format"] = m_sFormat;

        m_oFileBin.open(m_sFileBin, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        if (!m_oFileBin.is_open()) {
            throw FileError("Failed to create binary file");
        }
    }

    void InitializeArchives() {
        try {
            m_oArchiveSave = std::make_unique<SymEngine::RCPBasicAwareOutputArchive<
                cereal::PortableBinaryOutputArchive> >(m_oFileBin);
            m_oFileBin.seekg(0, std::ios::beg);
            m_oArchiveLoad = std::make_unique<SymEngine::RCPBasicAwareInputArchive<
                cereal::PortableBinaryInputArchive> >(m_oFileBin);
            m_oFileBin.seekp(m_lOffset);
        } catch (const std::exception &e) {
            throw FileError("Failed to initialize archives: " + std::string(e.what()));
        }
    }

    void SaveBookkeepingData() {
        try {
            debugPrint("Writing bookkeeping data to ", m_sFileJson);
            m_oFileJson["offsets"] = SerializeOffsets();
            m_oFileJson["meta"]["fileOffset"] = static_cast<Json::UInt64>(m_lOffset);
            m_oFileJson["meta"]["retId"] = m_lRetId;

            Json::StreamWriterBuilder builder;
            builder["commentStyle"] = "None";
            builder["indentation"] = "    ";

            std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
            std::ofstream outputFileStream(m_sFileJson);
            if (!outputFileStream.is_open()) {
                throw FileError("Failed to open JSON file for writing");
            }

            writer->write(m_oFileJson, &outputFileStream);
            debugPrint("Finished writing bookkeeping data");
        } catch (const std::exception &e) {
            debugPrint("Error saving bookkeeping data: ", e.what());
        }
    }

    bool DeleteFileIfExists(const std::string &path) {
        try {
            if (boost::filesystem::exists(path)) {
                return boost::filesystem::remove(path);
            }
            return true;
        } catch (const boost::filesystem::filesystem_error &e) {
            throw FileError("Filesystem error: " + std::string(e.what()));
        }
    }

    void Cleanup() {
        try {
            if (m_oFileBin.is_open()) {
                m_oFileBin.close();
            }
            m_mOffsets.clear();
            m_oFileJson.clear();
        } catch (...) {
            // Swallow exceptions in cleanup
        }
    }

    void _Append(size_t retId, const Types &... data) {
        try {
            auto p = m_oFileBin.tellp();
            if (!m_oFileBin.good()) {
                throw FileError("Binary file is in bad state");
            }

            m_mOffsets[retId].push_back(p);
            (*m_oArchiveSave)(data...);
            m_lOffset = m_oFileBin.tellp();

            debugPrint("Appended to retId: ", retId, ", element count: ", m_mOffsets[retId].size());
        } catch (const std::exception &e) {
            throw FileError("Failed to append data: " + std::string(e.what()));
        }
    }

    Json::Value SerializeOffsets() {
        try {
            std::lock_guard<std::mutex> lock(m_oMutexOffsets);
            Json::Value root;
            for (const auto &[retId, addrList]: m_mOffsets) {
                Json::Value tnJson;
                for (const auto &addr: addrList) {
                    tnJson.append(static_cast<Json::UInt64>(addr));
                }
                root[std::to_string(retId)] = tnJson;
            }
            return root;
        } catch (const std::exception &e) {
            throw FileError("Failed to serialize offsets: " + std::string(e.what()));
        }
    }

    void DeserializeOffsets(const Json::Value &root) {
        try {
            std::lock_guard<std::mutex> lock(m_oMutexOffsets);
            for (const auto &retId: root.getMemberNames()) {
                size_t retIdInt = std::stoull(retId);
                for (const auto &addrJson: root[retId]) {
                    m_mOffsets[retIdInt].push_back(static_cast<std::streampos>(addrJson.asUInt64()));
                }
            }
        } catch (const std::exception &e) {
            throw FileError("Failed to deserialize offsets: " + std::string(e.what()));
        }
    }

    template<typename... Args>
    void debugPrint(Args... args) {
        if (m_bDebug) {
            try {
                (std::cout << ... << args) << std::endl;
            } catch (...) {
                // Swallow debug print errors
            }
        }
    }
};
