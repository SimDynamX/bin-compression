#include "rlr_class.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

#define ERROR_MSG(msg) \
    std::cerr << msg << " OCCURED IN: " << '\n'; \
    std::cerr << "      File: " << __FILE__ << '\n'; \
    std::cerr << "      Function: " << __PRETTY_FUNCTION__ << '\n'; \
    std::cerr << "      Line: " << __LINE__ << '\n'; \

#define ERROR_MSG_AND_EXIT(msg) \
    std::cerr << msg << " OCCURED IN: " << '\n'; \
    std::cerr << "      File: " << __FILE__ << '\n'; \
    std::cerr << "      Function: " << __PRETTY_FUNCTION__ << '\n'; \
    std::cerr << "      Line: " << __LINE__ << std::endl; \
    std::exit(EXIT_FAILURE);

#define PRINT_DEBUG(msg) \
    std::cerr << msg << '\n'; \

//Constructors
RLR::RLR(){}


void RLR::Read_File(const std::filesystem::path& file_path, const int& number_of_bytes_to_read, const int& row) {
    // clear the binary data vector
    binary_data_vec.clear();


    std::ifstream input_file(file_path, std::ios::binary);
#ifdef DEBUG
    if(!input_file) {
        ERROR_MSG_AND_EXIT("Error: Unable to open the file.");
    }
#endif
    binary_data_vec.resize(number_of_bytes_to_read);
    // populate the binary data vector with the data from the file starting at number_of_bytes_to_read * row
    std::streampos start_position = static_cast<std::streampos>(number_of_bytes_to_read) * row;
    input_file.seekg(start_position);
#ifdef DEBUG
    if (input_file.fail()) {
        ERROR_MSG_AND_EXIT("Error: Unable to seek to the specified position in the file.");
    }
#endif
    input_file.close();
}

void RLR::Encode_With_One_Byte_Run_Length() {
    // Clear the encoded data vector
    encoded_data_vec.clear();

    const int data_type_size = this->Get_Data_Type_Size();
    encoded_data_vec.reserve(binary_data_vec.size() / 2);
    int byte_index = 0;
    std::vector<char> current_block;
    current_block.reserve(data_type_size);

    while (byte_index < binary_data_vec.size()) {
        uint8_t run_length = 1;
        int next_index = byte_index + data_type_size;

        current_block.assign(binary_data_vec.begin() + byte_index,
                                        binary_data_vec.begin() + byte_index + data_type_size);

        while ((next_index < binary_data_vec.size()) && run_length < 255) {
            if (std::equal(current_block.begin(), current_block.end(),
                           binary_data_vec.begin() + next_index)) {
                run_length++;
                next_index += data_type_size;
            } else {
                break;
            }
        }

        encoded_data_vec.push_back(static_cast<char>(run_length));
        encoded_data_vec.insert(encoded_data_vec.end(), current_block.begin(), current_block.end());

        byte_index = next_index;
    }

    encoded_data_vec.shrink_to_fit();
}

void RLR::Decode_With_One_Byte_Run_Length() {
    // Clear the vector to make sure we're starting fresh.
    decoded_data_vec.clear();

    decoded_data_vec.reserve(binary_data_vec.size());

    const int data_type_size = this->Get_Data_Type_Size();
    auto it = encoded_data_vec.begin();

    while(it != encoded_data_vec.end()) {
        uint8_t run_length = static_cast<uint8_t>(*it++);

        for (int run = 0; run < run_length; ++run) {
            decoded_data_vec.insert(decoded_data_vec.end(), it, it + data_type_size);
        }

        it += data_type_size;
    }
}

// why does this work?
void RLR::Encode_With_Two_Byte_Run_Length() {
    // Clear the encoded data vector
    encoded_data_vec.clear();

    int data_type_size = this->Get_Data_Type_Size();
    encoded_data_vec.reserve(binary_data_vec.size() / 2);
    int byte_index = 0;

    while (byte_index < binary_data_vec.size()) {
        uint16_t run_length = 1;
        int next_index = byte_index + data_type_size;

        std::vector<char> current_block(binary_data_vec.begin() + byte_index,
                                        binary_data_vec.begin() + byte_index + data_type_size);

        while ((next_index < binary_data_vec.size()) && run_length < 65535) {
            if (std::equal(current_block.begin(), current_block.end(),
                           binary_data_vec.begin() + next_index)) {
                run_length++;
                next_index += data_type_size;
            } else {
                break;
            }
        }

        encoded_data_vec.push_back(static_cast<char>(run_length >> 8));
        encoded_data_vec.push_back(static_cast<char>(run_length));
        encoded_data_vec.insert(encoded_data_vec.end(), current_block.begin(), current_block.end());

        byte_index = next_index;
    }

    encoded_data_vec.shrink_to_fit();
}

// why does this work?
void RLR::Decode_With_Two_Byte_Run_Length() {
    // Clear the vector to make sure we're starting fresh.
    decoded_data_vec.clear();

    decoded_data_vec.reserve(binary_data_vec.size());

    int data_type_size = this->Get_Data_Type_Size();
    auto it = encoded_data_vec.begin();

    while(it != encoded_data_vec.end()) {
        uint16_t run_length = static_cast<uint8_t>(*it++) << 8;
        run_length |= static_cast<uint8_t>(*it++);

        for (int run = 0; run < run_length; ++run) {
            decoded_data_vec.insert(decoded_data_vec.end(), it, it + data_type_size);
        }

        it += data_type_size;
    }
}

//FASTER
void RLR::Encode_With_One_Nibble_Run_Length() {
    encoded_data_vec.clear();

    const int data_type_size = this->Get_Data_Type_Size();
    encoded_data_vec.reserve(binary_data_vec.size() / 8);
    int byte_index = 0;
    std::vector<char> first_nibble_run_block = {0};
    first_nibble_run_block.reserve(data_type_size);
    std::vector<char> second_nibble_run_block = {0};
    second_nibble_run_block.reserve(data_type_size);

    while (byte_index < binary_data_vec.size()) {
        uint8_t first_nibble_run_length = 1;
        uint8_t second_nibble_run_length = 0;
        int next_index = byte_index + data_type_size;

        first_nibble_run_block.assign(binary_data_vec.begin() + byte_index, binary_data_vec.begin() + byte_index + data_type_size);

        while ((next_index < binary_data_vec.size()) && (first_nibble_run_length < 15) &&
                (std::equal(first_nibble_run_block.begin(), first_nibble_run_block.end(), binary_data_vec.begin() + next_index))) {
            first_nibble_run_length++;
            next_index += data_type_size;
        }

        byte_index = next_index;

        if (byte_index < binary_data_vec.size()) {
            second_nibble_run_block.assign(binary_data_vec.begin() + byte_index, binary_data_vec.begin() + byte_index + data_type_size);

            while ((next_index < binary_data_vec.size()) && (second_nibble_run_length < 15) &&
                        (std::equal(second_nibble_run_block.begin(), second_nibble_run_block.end(), binary_data_vec.begin() + next_index))) {
                second_nibble_run_length++;
                next_index += data_type_size;
            }
        }

        byte_index = next_index;

        encoded_data_vec.emplace_back(static_cast<char>((first_nibble_run_length << 4) | second_nibble_run_length));
        encoded_data_vec.insert(encoded_data_vec.end(), first_nibble_run_block.begin(), first_nibble_run_block.end());

        if (second_nibble_run_length) {
            encoded_data_vec.insert(encoded_data_vec.end(), second_nibble_run_block.begin(), second_nibble_run_block.end());
        }
    }

    encoded_data_vec.shrink_to_fit();
}

void RLR::Encode_With_One_Nibble_Run_Length_Squared(){
    // encoded_squared_data_vec.clear();

    // const int data_type_size = this->Get_Data_Type_Size();
    encoded_squared_data_vec.reserve(encoded_data_vec.size() / 2);
    int byte_index = 0;
    std::vector<char> first_nibble_run_block(1,0);
    first_nibble_run_block.reserve(1);
    std::vector<char> second_nibble_run_block = {0};
    second_nibble_run_block.reserve(1);

    while (byte_index < encoded_data_vec.size()) {
        uint8_t first_nibble_run_length = 1;
        uint8_t second_nibble_run_length = 0;
        int next_index = byte_index + 1;

        first_nibble_run_block.assign(encoded_data_vec.begin() + byte_index, encoded_data_vec.begin() + byte_index + 1);

        while ((next_index < encoded_data_vec.size()) && (first_nibble_run_length < 15) &&
                (std::equal(first_nibble_run_block.begin(), first_nibble_run_block.end(), encoded_data_vec.begin() + next_index))) {
            first_nibble_run_length++;
            next_index += 1;
        }

        byte_index = next_index;

        if (byte_index < encoded_data_vec.size()) {
            second_nibble_run_block.assign(encoded_data_vec.begin() + byte_index, encoded_data_vec.begin() + byte_index + 1);

            while ((next_index < encoded_data_vec.size()) && (second_nibble_run_length < 15) &&
                        (std::equal(second_nibble_run_block.begin(), second_nibble_run_block.end(), encoded_data_vec.begin() + next_index))) {
                second_nibble_run_length++;
                next_index += 1;
            }
        }

        byte_index = next_index;

        encoded_squared_data_vec.emplace_back(static_cast<char>((first_nibble_run_length << 4) | second_nibble_run_length));
        encoded_squared_data_vec.insert(encoded_squared_data_vec.end(), first_nibble_run_block.begin(), first_nibble_run_block.end());

        if (second_nibble_run_length) {
            encoded_squared_data_vec.insert(encoded_squared_data_vec.end(), second_nibble_run_block.begin(), second_nibble_run_block.end());
        }
    }

    encoded_squared_data_vec.shrink_to_fit();
}


//SLOWER
// void RLR::Encode_With_One_Nibble_Run_Length() {
//     encoded_data_vec.clear();

//     const int data_type_size = this->Get_Data_Type_Size();
//     encoded_data_vec.reserve(binary_data_vec.size());

//     size_t byte_index = 0;

//     while (byte_index < binary_data_vec.size()) {
//         const char* current_block = &binary_data_vec[byte_index];
//         uint8_t first_nibble_run_length = 1;
//         uint8_t second_nibble_run_length = 0;

//         while ((byte_index + first_nibble_run_length * data_type_size < binary_data_vec.size()) &&
//                (memcmp(current_block, &binary_data_vec[byte_index + first_nibble_run_length * data_type_size], data_type_size) == 0) &&
//                (first_nibble_run_length < 15)) {
//             ++first_nibble_run_length;
//         }

//         size_t current_block_end_index = byte_index + first_nibble_run_length * data_type_size;

//         // Calculate the second run length if there's more data
//         if (current_block_end_index < binary_data_vec.size()) {
//             current_block = &binary_data_vec[current_block_end_index];
//             while ((current_block_end_index + second_nibble_run_length * data_type_size < binary_data_vec.size()) &&
//                    (memcmp(current_block, &binary_data_vec[current_block_end_index + second_nibble_run_length * data_type_size], data_type_size) == 0) &&
//                    (second_nibble_run_length < 15)) {
//                 ++second_nibble_run_length;
//             }
//         }


//         uint8_t packed_run_lengths = (first_nibble_run_length << 4) | second_nibble_run_length;
//         encoded_data_vec.push_back(static_cast<char>(packed_run_lengths));

//         size_t new_size = encoded_data_vec.size() + data_type_size;
//         encoded_data_vec.resize(new_size);
//         memcpy(encoded_data_vec.data() + new_size - data_type_size, current_block, data_type_size);
//         byte_index += first_nibble_run_length * data_type_size;

//         if (second_nibble_run_length > 0) {
//             new_size = encoded_data_vec.size() + data_type_size;
//             encoded_data_vec.resize(new_size);
//             memcpy(encoded_data_vec.data() + new_size - data_type_size, current_block, data_type_size);
//             byte_index += second_nibble_run_length * data_type_size;
//         }
//     }


//     encoded_data_vec.shrink_to_fit();
// }

//FASTER
void RLR::Decode_With_One_Nibble_Run_Length() {
    decoded_data_vec.clear();
    decoded_data_vec.reserve(binary_data_vec.size());
    const int data_type_size = this->Get_Data_Type_Size();
    size_t byte_index = 0;

    while (byte_index < encoded_data_vec.size()) {

        uint8_t packed_run_lengths = static_cast<uint8_t>(encoded_data_vec[byte_index]);
        uint8_t first_nibble_run_length = packed_run_lengths >> 4;
        uint8_t second_nibble_run_length = packed_run_lengths & 0x0F;

        byte_index++;

        decoded_data_vec.insert(decoded_data_vec.end(), first_nibble_run_length * data_type_size,encoded_data_vec[byte_index]);
        byte_index += data_type_size;


        if ((second_nibble_run_length > 0) && (byte_index + data_type_size <= encoded_data_vec.size())) {
            decoded_data_vec.insert(decoded_data_vec.end(), second_nibble_run_length * data_type_size, encoded_data_vec[byte_index]);
            byte_index += data_type_size;
        }
    }
}

void RLR::Decode_With_One_Nibble_Run_Length_Squared() {
    decoded_squared_data_vec.clear();
    // decoded_squared_data_vec.reserve(binary_data_vec.size());
    // const int data_type_size = this->Get_Data_Type_Size();
    size_t byte_index = 0;

    while (byte_index < encoded_squared_data_vec.size()) {

        uint8_t packed_run_lengths = static_cast<uint8_t>(encoded_squared_data_vec[byte_index]);
        uint8_t first_nibble_run_length = packed_run_lengths >> 4;
        uint8_t second_nibble_run_length = packed_run_lengths & 0x0F;

        byte_index++;

        decoded_squared_data_vec.insert(decoded_squared_data_vec.end(), first_nibble_run_length * 1,encoded_squared_data_vec[byte_index]);
        byte_index += 1;


        if ((second_nibble_run_length > 0) && (byte_index + 1 <= encoded_squared_data_vec.size())) {
            decoded_squared_data_vec.insert(decoded_squared_data_vec.end(), second_nibble_run_length * 1, encoded_squared_data_vec[byte_index]);
            byte_index += 1;
        }
    }
}


//SLOWER
// void RLR::Decode_With_One_Nibble_Run_Length() {
//     decoded_data_vec.clear();

//     // Assuming the encoded data contains at least one data block after each byte of run lengths
//     const int data_type_size = this->Get_Data_Type_Size();
//     // Reserve some space upfront to reduce allocations
//     decoded_data_vec.reserve(encoded_data_vec.size() * data_type_size);

//     // Iterate through each byte of run length information
//     for (size_t i = 0; i < encoded_data_vec.size(); ) {
//         // Extract run lengths from the current byte
//         uint8_t packed_run_lengths = static_cast<uint8_t>(encoded_data_vec[i]);
//         uint8_t first_nibble_run_length = (packed_run_lengths >> 4) & 0x0F;  // Upper nibble
//         uint8_t second_nibble_run_length = packed_run_lengths & 0x0F;         // Lower nibble

//         // Move to the next index where the actual data starts
//         ++i;

//         // Handle the first run length's data block
//         for (int run = 0; run < first_nibble_run_length; ++run) {
//             decoded_data_vec.insert(decoded_data_vec.end(),
//                                     encoded_data_vec.begin() + i,
//                                     encoded_data_vec.begin() + i + data_type_size);
//         }

//         // Move past the data block we just processed
//         i += data_type_size;

//         // Handle the second run length's data block, if there is one
//         if (second_nibble_run_length > 0 && i < encoded_data_vec.size()) {
//             for (int run = 0; run < second_nibble_run_length; ++run) {
//                 decoded_data_vec.insert(decoded_data_vec.end(),
//                                         encoded_data_vec.begin() + i,
//                                         encoded_data_vec.begin() + i + data_type_size);
//             }
//             // Move past the second data block
//             i += data_type_size;
//         }
//     }
// }

//If this is too memory intensive, then break it up into chunks of 1024 bytes
void RLR::Encode_With_Burrow_Wheeler_Transformation_Little_Endian() {
    // Clear the encoded data vector
    encoded_data_vec.clear();

    std::vector<bool> data_type_sized_bytes_frequencey_vec(1ULL << (this->Get_Data_Type_Size() << 3), false);

    std::vector<char> data_type_sized_bytes_vec(this->Get_Data_Type_Size(), 0);
    std::vector<std::vector<char>> vec_of_data_type_sized_vec(binary_data_vec.size() + this->Get_Data_Type_Size());

    uint64_t data_type_sized_bytes_frequencey_vec_index = 0;
    for(int byte_index = 0; byte_index < binary_data_vec.size(); byte_index += this->Get_Data_Type_Size()){
        data_type_sized_bytes_vec.assign(binary_data_vec.begin() + byte_index, binary_data_vec.begin() + byte_index + this->Get_Data_Type_Size());
        for(int i = 0; i < data_type_sized_bytes_vec.size(); i++){
            data_type_sized_bytes_frequencey_vec_index |= (static_cast<uint64_t>(static_cast<uint8_t>(data_type_sized_bytes_vec[i])) << (8 * i));
        }
        data_type_sized_bytes_frequencey_vec[data_type_sized_bytes_frequencey_vec_index] = true;
        vec_of_data_type_sized_vec[byte_index / this->Get_Data_Type_Size()] = data_type_sized_bytes_vec;
    }

    bool sentinel_found = false;
    sentinel_vec.reserve(this->Get_Data_Type_Size());
    for(uint64_t i = 0; i < data_type_sized_bytes_frequencey_vec.size(); ++i) {
        if(!data_type_sized_bytes_frequencey_vec[i]) {
            sentinel_found = true;
            for(int j = 0; j < sentinel_vec.size(); ++j) {
                sentinel_vec[j] = static_cast<char>((i >> (8 * j)) & 0xFF);
                PRINT_DEBUG(std::string{"sentinel_vec["} + std::to_string(j) + std::string{"]: "} + std::to_string(sentinel_vec[j]));
            }
            break;
        }
    }

    if(!sentinel_found) {
        ERROR_MSG("Error: Unable to find a sentinel.");
        return;
    }

    vec_of_data_type_sized_vec.push_back(sentinel_vec);

    std::vector<std::vector<std::vector<char>>> rotations_vec;
    rotations_vec.reserve(vec_of_data_type_sized_vec.size());
    rotations_vec.push_back(vec_of_data_type_sized_vec);

    for(size_t rotation = 1; rotation < vec_of_data_type_sized_vec.size(); rotation++) {
        rotations_vec.push_back(std::vector<std::vector<char>>(vec_of_data_type_sized_vec.size()));
        // rotations_vec[rotation].insert(rotations_vec[rotation].end(), rotations_vec[rotation-1].begin() + 1, rotations_vec[rotation-1].end());
        // rotations_vec[rotation].insert(rotations_vec[rotation].end(), rotations_vec[rotation-1].begin(), rotations_vec[rotation-1].begin() + 1);
        // rotate rotations_vec[rotation] by 1 to the right and append to rotations_vec[rotation]
    }
    // Sort the rotations_vec
    std::sort(rotations_vec[0].begin(), rotations_vec[0].end());

    // Extract the last column from the sorted rotations_vec
    // for(size_t i = 0; i < rotations_vec.size(); i++) {
    //     for(size_t j = 0; j < rotations_vec[i][rotations_vec[i][0].size()].size(); j++) {
    //         encoded_data_vec.push_back(rotations_vec[i][rotations_vec[i].size()-1][j]);
    //         PRINT_DEBUG(std::string{"encoded_data_vec["} + std::to_string(encoded_data_vec.size()-1) + std::string{"]: "} + std::to_string(encoded_data_vec[encoded_data_vec.size()-1]));
    //     }
    // }

    // Extract the last column from each row of sorted rotations_vec
    for(size_t i = 0; i < rotations_vec.size(); i++) {
        for(size_t j = 0; j < rotations_vec[i][rotations_vec[i][0].size()].size(); j++) {
            encoded_data_vec.push_back(rotations_vec[i][rotations_vec[i].size()-1][j]);
            PRINT_DEBUG(std::string{"encoded_data_vec["} + std::to_string(encoded_data_vec.size()-1) + std::string{"]: "} + std::to_string(encoded_data_vec[encoded_data_vec.size()-1]));
        }
    }


    encoded_data_vec.shrink_to_fit();


}

void RLR::Decode_With_Inverse_Burrow_Wheeler_Transformation_Little_Endian(){
    // First, we'll recreate the table of rotations.
    std::vector<std::vector<char>> table(encoded_data_vec.size() / this->Get_Data_Type_Size(), std::vector<char>(this->Get_Data_Type_Size()));

    // Fill the table with the data from the BWT vector.
    for (size_t i = 0; i < encoded_data_vec.size(); i++) {
        table[i / this->Get_Data_Type_Size()][i % this->Get_Data_Type_Size()] = encoded_data_vec[i];
    }

    // Sort the table by each column, adding characters from the BWT in each iteration.
    for (size_t j = 0; j < this->Get_Data_Type_Size() - 1; j++) {
        // Insert BWT chars into the table.
        for (size_t i = 0; i < table.size(); i++) {
            table[i].insert(table[i].begin(), encoded_data_vec[i * this->Get_Data_Type_Size() + j % this->Get_Data_Type_Size()]);
        }
        // Sort the table.
        std::sort(table.begin(), table.end());
    }

    // Find the row that ends with the sentinel. This is the original data.
    decoded_data_vec.clear();
    for (const auto& row : table) {
        if (std::equal(row.end() - sentinel_vec.size(), row.end(), sentinel_vec.begin())) {
            decoded_data_vec.assign(row.begin(), row.end() - sentinel_vec.size());
            break;
        }
    }

    if (decoded_data_vec.empty()) {
        ERROR_MSG_AND_EXIT("Error: Unable to find the sentinel in the table of rotations.");
    }

    // Remove the sentinel from the end of the binary data vector.
    decoded_data_vec.erase(decoded_data_vec.end() - this->Get_Data_Type_Size(), decoded_data_vec.end());
}

//This may be a to guarentee to compress 1/8 of the data
//if we pack the data into 8 bit chunks, then we can use the same algorithm as above
void RLR::Encode_With_Move_To_Front_Transformation() {
    // Clear the encoded data vector
    encoded_data_vec.clear();

    std::vector<char> data_type_sized_bytes_vec(this->Get_Data_Type_Size(), 0);
    std::vector<std::vector<char>> vec_of_data_type_sized_vec(binary_data_vec.size() + this->Get_Data_Type_Size());

    uint64_t data_type_sized_bytes_vec_index = 0;
    for(int byte_index = 0; byte_index < binary_data_vec.size(); byte_index += this->Get_Data_Type_Size()){
        data_type_sized_bytes_vec.assign(binary_data_vec.begin() + byte_index, binary_data_vec.begin() + byte_index + this->Get_Data_Type_Size());
        vec_of_data_type_sized_vec[byte_index / this->Get_Data_Type_Size()] = data_type_sized_bytes_vec;
    }

    std::vector<char> alphabet(1ULL << (this->Get_Data_Type_Size() << 3), 0);
    for(int i = 0; i < alphabet.size(); i++){
        alphabet[i] = static_cast<char>(i);
    }

    for(int i = 0; i < vec_of_data_type_sized_vec.size(); i++){
        for(int j = 0; j < vec_of_data_type_sized_vec[i].size(); j++){
            for(int k = 0; k < alphabet.size(); k++){
                if(vec_of_data_type_sized_vec[i][j] == alphabet[k]){
                    encoded_data_vec.push_back(static_cast<char>(k));
                    alphabet.erase(alphabet.begin() + k);
                    alphabet.insert(alphabet.begin(), vec_of_data_type_sized_vec[i][j]);
                    break;
                }
            }
        }
    }

    encoded_data_vec.shrink_to_fit();
}

void RLR::Decode_With_Inverse_Move_To_Front_Transformation(){
    // Clear the decoded data vector
    decoded_data_vec.clear();

    std::vector<char> data_type_sized_bytes_vec(this->Get_Data_Type_Size(), 0);
    std::vector<std::vector<char>> vec_of_data_type_sized_vec(encoded_data_vec.size() + this->Get_Data_Type_Size());

    uint64_t data_type_sized_bytes_vec_index = 0;
    for(int byte_index = 0; byte_index < encoded_data_vec.size(); byte_index += this->Get_Data_Type_Size()){
        data_type_sized_bytes_vec.assign(encoded_data_vec.begin() + byte_index, encoded_data_vec.begin() + byte_index + this->Get_Data_Type_Size());
        vec_of_data_type_sized_vec[byte_index / this->Get_Data_Type_Size()] = data_type_sized_bytes_vec;
    }

    std::vector<char> alphabet(1ULL << (this->Get_Data_Type_Size() << 3), 0);
    for(int i = 0; i < alphabet.size(); i++){
        alphabet[i] = static_cast<char>(i);
    }

    for(int i = 0; i < vec_of_data_type_sized_vec.size(); i++){
        for(int j = 0; j < vec_of_data_type_sized_vec[i].size(); j++){
            for(int k = 0; k < alphabet.size(); k++){
                if(vec_of_data_type_sized_vec[i][j] == static_cast<char>(k)){
                    decoded_data_vec.push_back(alphabet[k]);
                    alphabet.erase(alphabet.begin() + k);
                    alphabet.insert(alphabet.begin(), static_cast<char>(k));
                    break;
                }
            }
        }
    }

    decoded_data_vec.shrink_to_fit();
}

void RLR::Write_Compressed_File(const std::filesystem::path& file_path) const {
    try {

        std::ofstream encoded_output_file(file_path, std::ios::binary | std::ios::app);

        encoded_output_file.write(encoded_data_vec.data(), encoded_data_vec.size());

        encoded_output_file.close();
    } catch (const std::exception& e) {
        ERROR_MSG_AND_EXIT(std::string{"Error: Unable to create the run-length encoded file.\n"} + std::string{"ERROR CODE: "} + std::string{e.what()});
    }
}

void RLR::Write_Decompressed_File(const std::filesystem::path& file_path) const {
    try{
        std::ofstream decoded_output_file(file_path, std::ios::binary | std::ios::app);

        decoded_output_file.write(decoded_data_vec.data(), decoded_data_vec.size());

        decoded_output_file.close();
    } catch (const std::exception& e) {
        ERROR_MSG_AND_EXIT(std::string{"Error: Unable to create the run-length decoded file.\n"} + std::string{"ERROR CODE: "} + std::string{e.what()});
    }
}

void RLR::Write_Compressed_File_Squared(const std::filesystem::path& file_path) const {
    try {

        std::ofstream encoded_output_file(file_path, std::ios::binary | std::ios::app);

        encoded_output_file.write(encoded_squared_data_vec.data(), encoded_squared_data_vec.size());

        encoded_output_file.close();
    } catch (const std::exception& e) {
        ERROR_MSG_AND_EXIT(std::string{"Error: Unable to create the run-length encoded file.\n"} + std::string{"ERROR CODE: "} + std::string{e.what()});
    }
}

void RLR::Write_Decompressed_File_Squared(const std::filesystem::path& file_path) const {
    try{
        std::ofstream decoded_output_file(file_path, std::ios::binary | std::ios::app);

        decoded_output_file.write(decoded_squared_data_vec.data(), decoded_squared_data_vec.size());

        decoded_output_file.close();
    } catch (const std::exception& e) {
        ERROR_MSG_AND_EXIT(std::string{"Error: Unable to create the run-length decoded file.\n"} + std::string{"ERROR CODE: "} + std::string{e.what()});
    }
}



//getters
const char* RLR::Get_Compression_Type() const {return compression_type;}

const std::vector<char> RLR::Get_Encoded_Data_Vec() const {return encoded_data_vec;}

const std::vector<char> RLR::Get_Decoded_Data_Vec() const {return decoded_data_vec;}

const std::vector<char> RLR::Get_Binary_Data_Vec() const {return binary_data_vec;}