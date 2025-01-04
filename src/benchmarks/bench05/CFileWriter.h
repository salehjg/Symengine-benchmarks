//
// Created by saleh on 12/29/24.
//

// This is a base class for CFileWriterX classes.

#pragma once

#include "CFileWriterBase.h"
#include "symengine/streaming_deserializer.h"
#include "symengine/streaming_serializer.h"

class CFileWriter : public CFileWriterBase {
protected:
    SymEngine::streaming_serializer se_serializer;

public:
    // Constructor that takes the file path to allocate
    explicit CFileWriter(const std::string& filePath)
        : CFileWriterBase(filePath) {

    }

    // Destructor: Print the partition sizes and offsets
    ~CFileWriter() {
    }

    // Create a new partition and return an ostream for it
    std::ostream& createPartition() {
        return CFileWriterBase::createPartition();
    }

    // Change the head of the stream to the start of a given partition index
    void seekToPartitionStart(int partitionIndex) {
        CFileWriterBase::seekToPartitionStart(partitionIndex);
    }

    // Print the partition sizes and offsets
    void printPartitionInfo() {
        CFileWriterBase::printPartitionInfo();
    }
};

