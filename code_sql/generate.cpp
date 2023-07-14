/**
 * Author: Dochot.be
 * Depot: Github ()
 * Encoding: UTF-8
 * Language: C++
 * Source(s):
 **/

#include <iostream>
#include <string>
#include "../resources/sha256.h"
#include "../resources/db/sqlite3.h"
#include "../resources/passwd-utils.hpp"
#include "../code_file/hash_R.hpp"
#include <time.h>
#include <signal.h>
#include <thread>
using namespace std;


/*Default value*/
static string DATABASE = "rainbow_table.db";
static int HASH_REDUCING_LENGTH = 15;
static int PWD_SIZE = 15;
static int NUMBER_OF_GENERATED_INSTANCES = 50000;
static bool LOG = false; //change this to print log in terminal


/*Value during process*/
sqlite3 *sql = NULL;
int counter_of_line  = 0;


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
        i2 = hash_R(i1, i, size_password, 0);
        if(LOG)
            ::printf(" %s -> (R_%i) %s  ", i1.c_str(), i,  i2.c_str());
    }
    if (LOG)
        cout << endl;
    return i2;
}


/**
 * Print the result of request in database
 */
int printDb(void *NotUsed, int argc, char **argv, char **azColName)
{
    if (LOG)
    {
        int i;
        for(i = 0; i<argc; i++)
            printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

        printf("\n");
    }
    return 0;
}


/**
 * Open sqlite db
 * @param name_db : path to stock db
 * @param db : pointer of db
 */
void openSqlite(string name_db, sqlite3 **db)
{
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(name_db.c_str(), db);
    sqlite3_enable_shared_cache(1);
    if( rc )
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
    else
        fprintf(stderr, "Opened database successfully\n");
}


/**
 * Close sqlite db
 * @param db : pointer of db
 */
void closeSqlite(sqlite3 *db)
{
    sqlite3_close(db);
    cout << "Close DataBase !" << endl;
}


/**
 * Create a table in db
 * @param db : pointer of db
 */
void createTable(sqlite3 *db)
{
    int exit = 0; char* messageError;
    std::string sql = "CREATE TABLE IF NOT EXISTS DICO(" \
                      "PASSWORD   TEXT   PRIMARY KEY   NOT NULL," \
                      "HASH       TEXT                  NOT NULL );";
    exit = sqlite3_exec(db, sql.c_str(), NULL, 0, &messageError);
    if (exit != SQLITE_OK)
    {
        std::cerr << "Error Create Table " << messageError << std::endl;
        sqlite3_free(messageError);
    }
    else
        std::cout << "Table created Successfully" << std::endl;
}


/**
 * Add data in dataBase
 * @param passWord : clear word
 * @param hashLast : password after multiple hash and reduce
 * @param db : pointer of db
 */
void addData(string passWord, string hashLast, sqlite3 *db)
{

    char *zErrMsg = 0; int rc;
    string sql = "INSERT INTO DICO (PASSWORD, HASH) VALUES ('" + passWord + "','" + hashLast + "');";
    rc = sqlite3_exec(db, sql.c_str(), printDb, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        ++counter_of_line;
        if (LOG)
            fprintf(stdout, "Records created successfully : %s\n", sql.c_str());
    }
}


/***/
void generate_list_size(int min, int max, int* list)
{
    for (int i = 0; i <= max-min; ++i)
        list[i] = min + i;
}


/**
 * Thread to launch
 * @param size : password size
 * @param reduce : number of reduce
 * @param db : link to db
 */
void thread_fct(int size, int reduce, sqlite3 *db)
{
    string clear = rainbow::generate_passwd(size);
    string last  = createFinalHash(clear, size, reduce);
    addData(clear, last, db);
}


/**
 * Generate the table with password and "transform" hash
 * @param number_password : number of instance
 * @param number_reduction : number of hash and reduce
 * @param size_password : the number of character in password
 * @param db : pointer of db
 */
void generate(int number_password, int number_reduction, int size_password_min, int size_password_max, sqlite3 *db)
{
    int size_diff = size_password_max-size_password_min+1;
    int list[size_diff];
    generate_list_size(size_password_min, size_password_max, list);
    string clear; string last;
    for (int i = 0; i <= number_password; ++i)
    {
        if ( LOG )
            ::printf("SIZE: %i\n", list[i%size_diff]);
        thread th(thread_fct, list[i%size_diff], number_reduction, db);
        th.join();
        //clear = rainbow::generate_passwd(list[i%size_diff]);//TODO
        //last  = createFinalHash(clear, list[i%size_diff], number_reduction);//TODO
        //addData(clear, last, db);
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
    closeSqlite(sql);
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
 *               size of password /
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
    openSqlite(dataBase,&sql);
    createTable(sql);
    generate(generates, reduction, size_min, size_max, sql);
    quitProgram();

    return 1;
}