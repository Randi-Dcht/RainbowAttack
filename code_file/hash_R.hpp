/**
 * Author: Dochot.be
 * Depot: Github
 * Encoding: UTF-8
 * Language: C/C++
 * Source(s): - https://github.com/jfmengels/rainbowtable
 *            - https://github.com/alahyaoui/RainbowAttack/blob/main/utils/reduction.hpp
 *            - https://stackoverflow.com/questions/26670494/reducing-algorithm-for-sha256-rainbow-tables
 **/

#ifndef RAINBOWATTACK_HASH_R_H
#define RAINBOWATTACK_HASH_R_H
#define SIZE 16
#include <stdio.h>
#include <string>
#include <math.h>
#include <iostream>

const char * WORDS_DICTIONARY = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                     "abcdefghijklmnopqrstuvwxyz"
                     "0123456789";
const int WORD_SIZE = 62;

/**
 * @author : Jeroen Engels
 * SOURCE  : https://github.com/jfmengels/rainbowtable/blob/master/RainbowTable.cpp
 * LICENCE : no license
 */
void toHex(const char *text, unsigned char *bytes)
{
    unsigned int temp;
    for (int i = 0; i < SIZE; i++)
    {
        std::sscanf(text + 2 * i, "%2x", &temp);
        bytes[i] = temp;
    }
}


/**
 * Use image to hash word (call method of other class)
 * @param str : hash to reduce
 * @param R_i : number of function
 * @param size_password : size of password
 * @param n_modulo : arbitrary modulo
 */
std::string hash_R(const std::string hash_to_reduce, int number_of_function, int size_of_password, int n_modulo)
{
    std::string reduction;
    unsigned char bytes[SIZE];
    toHex(hash_to_reduce.c_str(), bytes);
    for (unsigned i = 0; i < size_of_password; i++)
        reduction += WORDS_DICTIONARY[bytes[(i + number_of_function) % SIZE] % WORD_SIZE];
    return reduction;
}

#endif //RAINBOWATTACK_HASH_R_H
