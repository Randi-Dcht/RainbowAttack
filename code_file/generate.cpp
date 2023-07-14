/**
 * Author: Dochot.be
 * Depot: Github
 * Encoding: UTF-8
 * Language: C++
 * Source(s):
 **/

#include <iostream>
#include <string>
#include "../resources/sha256.h"
#include "../resources/passwd-utils.hpp"
#include "hash_R.hpp"
#include <time.h>
#include <signal.h>
using namespace std;


/*Default value*/
static string DATABASE = "rainbow_table.db";
static int HASH_REDUCING_LENGTH = 15;
static int PWD_SIZE = 15;
static int NUMBER_OF_GENERATED_INSTANCES = 50000;
static bool LOG = false; //change this to print log in terminal


/**Value in db*/
int counter_of_line  = 0;
/**file of db*/
ofstream file_open;


/**
 * Use hash and image to generate last hash to stock in db
 * @param word_clear : password to hash and reduce
 * @param size_password : number of character in password
 * @param number_Reduce : number of hash and reduce
 */
string createFinalHash(string word_clear, int size_password, int number_Reduce)
{
    string  i1;string  i2 = word_clear;
    if (LOG)
        ::printf("%s ->", word_clear.c_str());
    for (int i = 1; i <= number_Reduce; ++i)
    {
        if (LOG)
            ::printf("->");
        i1 = sha256(i2);
        i2 = hash_R(i1, i, size_password, 16);
        if(LOG)
            ::printf(" %s -> (R_%i) %s  ", i1.c_str(), i,  i2.c_str());
    }
    if (LOG)
        cout << endl;
    return i2;
}


/**
 * Open sqlite db
 * @param name_db : path to stock db
 */
ofstream openSqlite(string name_db)
{
    ofstream file_to_open(name_db, ios::app);
    if( !file_to_open.is_open() )
        cout << "Error to open file !!!" << endl;
    return file_to_open;
}


/**
 * Close sqlite db
 */
void closeSqlite()
{
    file_open.close();
    cout << "Close DataBase !" << endl;
}


/**
 * Add data in dataBase
 * @param passWord : clear word
 * @param hashLast : password after multiple hash and reduce
 */
void addData(string passWord, string hashLast)
{
    file_open  << hashLast << "|" << passWord << endl;
    ++counter_of_line;
}


/**
 * Create a list with all size
 * @param min : minimal number in list
 * @param max : maximal number in list
 * @param list : pointer to list
 */
void generate_list_size(int min, int max, int* list)
{
    for (int i = 0; i <= max-min; ++i)
        list[i] = min + i;
}


/**
 * Generate the table with password and "transform" hash
 * @param number_password : number of instance
 * @param number_reduction : number of hash and reduce
 * @param size_password : the number of character in password
 */
void generate(int number_password, int number_reduction, int size_password_min, int size_password_max)
{
    int size_diff = size_password_max-size_password_min+1;
    int list[size_diff];
    generate_list_size(size_password_min, size_password_max, list);
    string clear; string last;
    for (int i = 0; i <= number_password; ++i)
    {
        if ( LOG )
            ::printf("SIZE: %i\n", list[i%size_diff]);
        clear = rainbow::generate_passwd(list[i%size_diff]);
        last  = createFinalHash(clear, list[i%size_diff], number_reduction);
        addData(clear, last);
    }
}


/**
 * Print the local time
 * @param msg : message to print
 */
void printTime(string msg)
{
    time_t currentime;
    time(&currentime);
    ::printf("%s at %s", msg.c_str() ,ctime(&currentime));
}


/**
 * Process to quit
 */
void quitProgram()
{
    closeSqlite();
    ::printf("Line add : %i \n", counter_of_line);
    printTime("End");
}


/**
 * Launch this method when CTRL+C is press
 * @param signum : signal number
 */
void handler_stopProcess(int signum)
{
    cout << "Quit Process : success" << endl;
    quitProgram();
    exit(signum);
}


/**
 * Main to launch generate the db
 * @param argv : number of instances /
 *               number of reduce /
 *               size of password (min) /
 *               size of password (max) /
 *               path to stock db
 *  @Return : sqlite db with head(password) and teal(hash/reduce)
 */
int main(int argc, char **argv)
{
    signal(SIGINT, handler_stopProcess);

    int generates,reduction,size_min, size_max = 0;
    string dataBase;
    if (argc == 6)
    {
        generates = ::strtol(argv[1], NULL, 10);
        reduction = ::strtol(argv[2], NULL, 10);
        size_min  = ::strtol(argv[3], NULL, 10);
        size_max  = ::strtol(argv[4], NULL, 10);
        dataBase  = argv[5];
    }
    else
    {
        cout << "WARNING : default value to generate ";
        cout << " -INT[number_to_generate]  -INT[number_of_reduce]  -INT[size_password_min] -INT[size_password_max] -STR[path_db]" << endl;
        generates = NUMBER_OF_GENERATED_INSTANCES;
        reduction = HASH_REDUCING_LENGTH;
        size_min  = PWD_SIZE;
        size_max  = PWD_SIZE;
        dataBase  = DATABASE;
    }

    ::printf("Values : Generates->%i | Reduces->%i | Sizes->%i & %i | file->%s \n",generates, reduction, size_min, size_max, dataBase.c_str());

    cout << "Generate DataBase" << endl;
    cout << "You can quit with CTRL+C ;)" << endl;


    printTime("Start");
    file_open = openSqlite(dataBase);
    generate(generates, reduction, size_min, size_max);
    quitProgram();

    return 1;
}