#include "lzw_class.h"

//Constructors
LZW_Stats::LZW_Stats() :
    avgSizeBytes(0.0), avgEncodedTimeMs(0.0), avgDecodedTimeMs(0.0),
    avgCompressionRatio(0.0), avgPeakMemoryDuringEncoding(0), avgPeakMemoryDuringDecoding(0),
    avgEncodedThroughput(0.0), avgThroughputDecoded(0.0) {}

LZW_Stats::LZW_Stats(double avgSizeBytes, double avgEncodedTimeMs, double avgDecodedTimeMs, double avgCompressionRatio, size_t avgPeakMemoryDuringEncoding, size_t avgPeakMemoryDuringDecoding, double avgEncodedThroughput, double avgThroughputDecoded) :
    avgSizeBytes(avgSizeBytes), avgEncodedTimeMs(avgEncodedTimeMs), avgDecodedTimeMs(avgDecodedTimeMs),
    avgCompressionRatio(avgCompressionRatio), avgPeakMemoryDuringEncoding(avgPeakMemoryDuringEncoding), avgPeakMemoryDuringDecoding(avgPeakMemoryDuringDecoding),
    avgEncodedThroughput(avgEncodedThroughput), avgThroughputDecoded(avgThroughputDecoded) {}

// Space Complexity:
// Best: O(n) for completely repetitive data.
// Average:O(n) for the dictionary plus the output.
// Worst:O(n) for the dictionary plus the output.
std::vector<int> LZW_Stats::lzwEncode(const std::vector<char>& input) {
    std::unordered_map<std::string, int> dictionary;
    std::vector<int> lzwEncoded;
    lzwEncoded.reserve(input.size());  // Reduce potential reallocations

    // Initialize the dictionary with single-character entries
    for (int i = 0; i < 256; ++i) {
        dictionary[std::string(1, static_cast<char>(i))] = i;
    }

    std::string current = "";
    for (char c : input) {
        current += c;
        if (dictionary.find(current) == dictionary.end()) {
            current.pop_back();  // Remove last character
            lzwEncoded.push_back(dictionary[current]);
            dictionary[current + c] = dictionary.size();
            current = c;
        }
    }

    if (!current.empty()) {
        lzwEncoded.push_back(dictionary[current]);
    }

    return lzwEncoded;
}

// Function to perform LZW decoding
// Time Complexity:
// Best, Average, Worst:O(n)Processing each code in the input requires constant
// time since the dictionary is built as the decoding progresses.
// Space Complexity:
// Best, Average, Worst:O(n) The dictionary's size grows with the input, plus the output size.


std::vector<char> LZW_Stats::lzwDecode(const std::vector<int>& input) {
    if (input.empty()) {
        return {};
    }

    std::unordered_map<int, std::string> dictionary;
    std::vector<char> lzwDecoded;
    lzwDecoded.reserve(input.size());  // Reduce potential reallocations

    // Initialize the dictionary with single-character entries
    for (int i = 0; i < 256; ++i) {
        dictionary[i] = std::string(1, static_cast<char>(i));
    }

    std::string current = dictionary.at(input[0]);
    lzwDecoded.insert(lzwDecoded.end(), current.begin(), current.end());

    for (size_t i = 1; i < input.size(); ++i) {
        int code = input[i];
        std::string entry;

        if (dictionary.count(code)) {
            entry = dictionary[code];
        } else if (code == dictionary.size()) {
            entry = current + current[0];
        } else {
            throw std::runtime_error("LZW decoding error: Invalid code.");
        }

        lzwDecoded.insert(lzwDecoded.end(), entry.begin(), entry.end());

        // Add a new entry to the dictionary
        dictionary[dictionary.size()] = current + entry[0];

        current = entry;
    }

    return lzwDecoded;
}

// Functions
void LZW_Stats::printStats() {
    std::cout << "LZW: Average size in bytes: " << avgSizeBytes << std::endl;
    std::cout << "LZW: Average encoded time in ms: " << avgEncodedTimeMs << std::endl;
    std::cout << "LZW: Average decoded time in ms: " << avgDecodedTimeMs << std::endl;
    std::cout << "LZW: Average compression ratio: " << avgCompressionRatio << std::endl;
    std::cout << "LZW: Average peak memory during encoding: " << avgPeakMemoryDuringEncoding << std::endl;
    std::cout << "LZW: Average peak memory during decoding: " << avgPeakMemoryDuringDecoding << std::endl;
    std::cout << "LZW: Average encoded throughput: " << avgEncodedThroughput << std::endl;
    std::cout << "LZW: Average throughput decoded: " << avgThroughputDecoded << std::endl;
}

void LZW_Stats::calculateAvgStats(int divisor){
    avgSizeBytes /= divisor;
    avgEncodedTimeMs /= divisor;
    avgDecodedTimeMs /= divisor;
    avgCompressionRatio /= divisor;
    avgPeakMemoryDuringEncoding /= divisor;
    avgPeakMemoryDuringDecoding /= divisor;
    avgEncodedThroughput /= divisor;
    avgThroughputDecoded /= divisor;
}

void LZW_Stats::getFileStats(std::vector<char> &binaryData, const char* lzwEncodedFileName, const char* lzwDecodedFileName, size_t fileSize){
        auto startEncodelzw = std::chrono::high_resolution_clock::now();
        std::vector<int> lzwEncoded = lzwEncode(binaryData);
        auto stopEncodelzw = std::chrono::high_resolution_clock::now();
        auto durationEncodelzw = std::chrono::duration_cast<std::chrono::milliseconds>(stopEncodelzw - startEncodelzw);

        // Perform lzw decoding
        auto startDecodelzw = std::chrono::high_resolution_clock::now();
        std::vector<char> lzwDecoded = lzwDecode(lzwEncoded);
        auto stopDecodelzw = std::chrono::high_resolution_clock::now();
        auto durationDecodelzw = std::chrono::duration_cast<std::chrono::milliseconds>(stopDecodelzw - startDecodelzw);

        // Calculate the peak memory usage
        // avglzwStats.setAvgPeakMemoryDuringDecoding(avglzwStats.getAvgPeakMemoryDuringDecoding() + getPeakMemoryUsage());
        // avglzwStats.setAvgPeakMemoryDuringEncoding(avglzwStats.getAvgPeakMemoryDuringEncoding() + getPeakMemoryUsage());

        // // Calculate the throughput for encoding and decoding
        avgEncodedThroughput += static_cast<double>(fileSize) / durationEncodelzw.count() * 1000; // bytes per second
        avgThroughputDecoded += static_cast<double>(fileSize) / durationDecodelzw.count() * 1000; // bytes per second

        // Verify that no data is lost by comparing decoded data with the original data
        bool lzwDataMatches = binaryData == lzwDecoded;
        std::cout << "lzw Data Matches: " << (lzwDataMatches ? "Yes" : "No") << std::endl;

        // Create a binary file from lzw encoded data for further verification
        std::ofstream lzwOutFile(lzwEncodedFileName, std::ios::binary);
        std::ofstream lzwDecodedOutFile(lzwDecodedFileName, std::ios::binary);
        if (!lzwOutFile || !lzwDecodedOutFile) {
            std::cerr << "Error: Unable to create the lzw encoded or decoded file." << std::endl;
        }

        for (size_t i = 0; i < lzwEncoded.size(); ++i) {
            int code = lzwEncoded[i];
            lzwOutFile.write(reinterpret_cast<const char*>(&code), sizeof(int));
        }

        for (size_t i = 0; i < lzwDecoded.size(); ++i) {
            char byte = lzwDecoded[i];
            lzwDecodedOutFile.write(reinterpret_cast<const char*>(&byte), sizeof(char));
        }

        lzwOutFile.close();
        lzwDecodedOutFile.close();

        // open the lzw encoded file and determine the file size
        avgSizeBytes += getFileSize(lzwEncodedFileName);
        avgEncodedTimeMs += durationEncodelzw.count();
        avgDecodedTimeMs += durationDecodelzw.count();
        avgCompressionRatio += static_cast<double>(getFileSize(lzwEncodedFileName)) / fileSize;

}

 void LZW_Stats::getStatsFromEncodingDecodingFunctions(const char* filename, int numIterations) {
    std::cout << "Compressing " << filename << " using LZW " << std::endl;
    const char* lzwEncodedFilename = "lzw_encoded.bin";
    const char* lzwDecodedFilename = "lzw_decoded.bin";

     LZW_Stats avgLZWStats;

        for (int i = 0; i < numIterations; ++i) {
        // Read the binary file
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) {
            std::cerr << "Error: Unable to open the file for reading." << std::endl;
        }

        inFile.seekg(0, std::ios::end);
        size_t fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);

        std::vector<char> binaryData(fileSize);
        inFile.read(binaryData.data(), fileSize);
        inFile.close();

        // Perform LZW encoding and decoding
        avgLZWStats.getFileStats(binaryData, lzwEncodedFilename, lzwDecodedFilename, fileSize);
    }

    // Calculate the average stats for the current file
    avgLZWStats.calculateAvgStats(numIterations);

    avgSizeBytes += avgLZWStats.getAvgSizeBytes();
    avgEncodedTimeMs += avgLZWStats.getAvgEncodedTimeMs();
    avgDecodedTimeMs += avgLZWStats.getAvgDecodedTimeMs();
    avgCompressionRatio += avgLZWStats.getAvgCompressionRatio();
    avgPeakMemoryDuringEncoding += avgLZWStats.getAvgPeakMemoryDuringEncoding();
    avgPeakMemoryDuringDecoding += avgLZWStats.getAvgPeakMemoryDuringDecoding();
    avgEncodedThroughput += avgLZWStats.getAvgEncodedThroughput();
    avgThroughputDecoded += avgLZWStats.getAvgThroughputDecoded();
 }

// Setters
void LZW_Stats::setAvgSizeBytes(double value) { avgSizeBytes = value; }
void LZW_Stats::setAvgEncodedTimeMs(double value) { avgEncodedTimeMs = value; }
void LZW_Stats::setAvgDecodedTimeMs(double value) { avgDecodedTimeMs = value; }
void LZW_Stats::setAvgCompressionRatio(double value) { avgCompressionRatio = value; }
void LZW_Stats::setAvgPeakMemoryDuringEncoding(size_t value) { avgPeakMemoryDuringEncoding = value; }
void LZW_Stats::setAvgPeakMemoryDuringDecoding(size_t value) { avgPeakMemoryDuringDecoding = value; }
void LZW_Stats::setAvgEncodedThroughput(double value) { avgEncodedThroughput = value; }
void LZW_Stats::setAvgThroughputDecoded(double value) { avgThroughputDecoded = value; }

// Getters
double LZW_Stats::getAvgSizeBytes() const { return avgSizeBytes; }
double LZW_Stats::getAvgEncodedTimeMs() const { return avgEncodedTimeMs; }
double LZW_Stats::getAvgDecodedTimeMs() const { return avgDecodedTimeMs; }
double LZW_Stats::getAvgCompressionRatio() const { return avgCompressionRatio; }
size_t LZW_Stats::getAvgPeakMemoryDuringEncoding() const { return avgPeakMemoryDuringEncoding; }
size_t LZW_Stats::getAvgPeakMemoryDuringDecoding() const { return avgPeakMemoryDuringDecoding; }
double LZW_Stats::getAvgEncodedThroughput() const { return avgEncodedThroughput; }
double LZW_Stats::getAvgThroughputDecoded() const { return avgThroughputDecoded; }