#pragma once

#include <cstddef>
#include <iostream>
#include <cassert> // Add this line to include the assert function
#include <fstream>
#include <string>
#include "../functions/file_functions.h"

class RLR_Stats {
public:
    // Constructors
    RLR_Stats();
    RLR_Stats(double avgSizeBytes, double avgEncodedTimeMs, double avgDecodedTimeMs,
            double avgCompressionRatio, size_t avgPeakMemoryDuringEncoding,
            size_t avgPeakMemoryDuringDecoding, double avgEncodedThroughput, double avgThroughputDecoded);




    /**
     * The function `encode` takes a vector of characters as input and returns a vector of pairs,
     * where each pair represents a character and its run length in the input vector.
     *
     * @param input A vector of characters that represents the input sequence.
     *
     * @return The function `encode` returns a `std::vector<std::pair<char, uint8_t>>`.
     */
    std::vector<std::pair<char, uint8_t>> encode(const std::vector<char>& input, int data_type_size);

    std::vector<std::pair<char, uint8_t>>  encodeRow(const std::vector<char>& input, int row, int data_type_size);

    /**
     * The function `decode` takes a vector of pairs representing characters and their
     * corresponding run lengths, and returns a vector of characters where each character is repeated
     * according to its run length.
     *
     * @param input A vector of pairs, where each pair consists of a character and an integer. The
     * character represents a repeated sequence of characters, and the integer represents the number of
     * times the character is repeated.
     *
     * @return The function `decode` returns a `std::vector<char>` containing the decoded sequence
     * of characters.
     */
    std::vector<char> decode(const std::vector<std::pair<char, uint8_t>>& input, int data_type_size);
    std::vector<char> decodeRow(const std::vector<std::pair<char, uint8_t>>& input, int row, int data_type_size);

    //functions
    void printStats();
    void calculateAvgStats(int divisors);
    void getFileStats(std::vector<char> &binaryData, const char* encodedFilename, const char* decodedFilename, size_t fileSize, std::filesystem::path& currentDir);
    void getStatsFromEncodingDecodingFunctions(const char* filename, int numIterations, std::filesystem::path& currentDir);

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
    double avgSizeBytes;
    double avgEncodedTimeMs;
    double avgDecodedTimeMs;
    double avgCompressionRatio;
    size_t avgPeakMemoryDuringEncoding;
    size_t avgPeakMemoryDuringDecoding;
    double avgEncodedThroughput;
    double avgThroughputDecoded;
};