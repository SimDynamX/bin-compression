#pragma once

#include <cstddef>
#include <iostream>
#include <vector>
#include <cstdint>
#include "../functions/file_functions.h"

class Binary_Interpolation_Stats {
    public:
        // Constructors
        Binary_Interpolation_Stats();
        Binary_Interpolation_Stats(double avgSizeBytes, double avgEncodedTimeMs, double avgDecodedTimeMs,
                double avgCompressionRatio, size_t avgPeakMemoryDuringEncoding,
                size_t avgPeakMemoryDuringDecoding, double avgEncodedThroughput, double avgThroughputDecoded);

        std::vector<char> encode(const std::vector<char>& input);
        std::vector<char> decode(const std::vector<char>& input, int length);

        // Functions
        void printStats();
        void calculateAvgStats(int divisors);
        void getFileStats(std::vector<char> &binaryData, const char* runLengthFilename, const char* runLengthDecodedFilename, size_t fileSize);
        void getStatsFromEncodingDecodingFunctions(const char* filename, int numIterations);

        // Setters
        void setAvgSizeBytes(double value);
        void setAvgEncodedTimeMs(double value);
        void setAvgDecodedTimeMs(double value);
        void setAvgCompressionRatio(double value);
        void setAvgPeakMemoryDuringEncoding(size_t value);
        void setAvgPeakMemoryDuringDecoding(size_t value);
        void setAvgEncodedThroughput(double value);
        void setAvgThroughputDecoded(double value);

        // Getters
        double getAvgSizeBytes() const;
        double getAvgEncodedTimeMs() const;
        double getAvgDecodedTimeMs() const;
        double getAvgCompressionRatio() const;
        size_t getAvgPeakMemoryDuringEncoding() const;
        size_t getAvgPeakMemoryDuringDecoding() const;
        double getAvgEncodedThroughput() const;
        double getAvgThroughputDecoded() const;

        

    private:
        // Stats attributes
        double avgSizeBytes;
        double avgEncodedTimeMs;
        double avgDecodedTimeMs;
        double avgCompressionRatio;
        size_t avgPeakMemoryDuringEncoding;
        size_t avgPeakMemoryDuringDecoding;
        double avgEncodedThroughput;
        double avgThroughputDecoded;



};