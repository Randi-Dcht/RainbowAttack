/**
 * Author: Dochot.be
 * Depot: Github ()
 * Encoding: UTF-8
 * Language: C 
 * Source(s):
 **/


#include <iostream>
#include <string>
#include "../resources/sha256.h"
#include "../resources/db/sqlite3.h"
#include "../resources/passwd-utils.hpp"
#include "../code_file/hash_R.hpp"
using namespace std;

static bool  LOG = false;

/*value during execution*/
string hash_to_search = "hash_none";
string password_crack = "no-crack";

//TODO this parti is duplicated
static int resultDb(void *NotUsed, int argc, char **argv, char **azColName)//TODO same
{
    password_crack = argv[0];
    ::printf("A value is find : %s for %s\n", argv[0], hash_to_search.c_str());
    return 0;
}
void openSqlite(string name_db, sqlite3 **db)//TODO same
{
    int rc;
    rc = sqlite3_open(name_db.c_str(), db);
    if( rc )
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
    else
        fprintf(stderr, "Opened database successfully\n");
}
void closeSqlite(sqlite3 *db)//TODO same
{
    sqlite3_close(db);
    cout << "Close DataBase !" << endl;
}


/**
 * Research of last reduce  in db
 * @param reduceToCheck : reduce
 */
void checkReduce(sqlite3 *db, const string& reduceToCheck)
{

    char *zErrMsg = 0;
    int rc;
    string sql;
    const char* data = "Callback function called";
    sql = "SELECT * from DICO WHERE HASH='" + reduceToCheck + "';";
    rc = sqlite3_exec(db, sql.c_str(), resultDb, (void*)data, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}


/**
 * Transform hash to reduce, to find the last reduce
 * @param hash_ : hash to crack
 * @param id_of_reduce : actual reduce to check
 * @param number_of_reduce : maximum reduce
 * @param size_password : number of character in password
 */
void hashCheck(sqlite3 *db, const string& hash_, int id_of_reduce, int number_of_reduce, int size_password)
{
    string j1 = hash_R(hash_, id_of_reduce, size_password, 0);
    if (LOG)
        ::printf("(R_%i) %s ", id_of_reduce, j1.c_str());
    string  j2;
    for (int i = id_of_reduce+1; i <= number_of_reduce; ++i)
    {
        j2 = sha256(j1);
        j1 = hash_R(j2, i, size_password, 0);
        if (LOG)
            ::printf(" -> %s -> (R_%i) %s", j2.c_str(), i, j1.c_str());
    }
    if (LOG)
        cout << endl;
    checkReduce(db, j1);
}


/**
 * Read file with hash to crack
 * @param path_file : path to file
 * @param reduce : maximum reduce
 * @param size_pwd : number of character in password
 */
void read_pwdHash(sqlite3 *db, const string& path_file, const string& path_ouput, int reduce, int size_pwd)
{
    ifstream file(path_file);
    ofstream outf(path_ouput);
    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            hash_to_search = line;
            for (int j = reduce; j > 0; --j)//TODO this in thread
                hashCheck(db, line, j, reduce, size_pwd);
            outf << password_crack << endl;
            password_crack = "no-crack";
        }
        file.close();
        outf.close();
    }
}


/**
 * Count number of password is cracked
 * @param file_crack : path to file
 * @param file_pwd_clear : path to file
 */
void statistic_attack(const string& file_crack, const string& file_pwd_clear)
{
    ifstream crack(file_crack);
    ifstream clear(file_pwd_clear);

    if (crack.is_open() && clear.is_open())
    {
        string lineA, lineC; int count = 0; int target = 0;
        while (getline(crack, lineA) && getline(clear, lineC))
        {
            ++count;
            if (lineA == lineC)
                ++target;
        }
        float percent = ((float)target/(float)count)*100;
        ::printf("There are %i passwords and %i passwords is cracked (%.2f percents)\n", count, target,percent);
    }
}


/**
 * Launch a rainbow attack
 */
int main(int argc, char **argv)
{
    int reduction = 0;
    string dataBase,pathOut,pathIn;
    string pathVerif = "none";
    if (argc >= 5)
    {
        dataBase  = argv[1];
        pathIn    = argv[2];
        pathOut   = argv[3];
        reduction = ::strtol(argv[4], NULL, 10);
        if (argc == 6)
            pathVerif = argv[5];
    }
    else
    {
        cout << "ERROR : problem with arguments\n";
        cout << " -STR[path_to_db] -STR[path_hash_IN] -STR[path_decode_OUT] -INT[reduce] [OPTIONAL: -STR[path_check_decode]]" << endl;
       exit(0);
    }

    ::printf("DB->%s | FILE_IN-> %s | FILE_OUT->%s | REDUCTION->%i | FILE_CHECK->%s\n",dataBase.c_str(), pathIn.c_str(), pathOut.c_str(), reduction, pathVerif.c_str());

    cout << "Launch Rainbow attack !" << endl;
    sqlite3 *sql = NULL;

    string file_hash_input;
    string file_pwd_output;
    string file_pwd_check;

    openSqlite(dataBase, &sql);
    read_pwdHash(sql,pathIn,pathOut, reduction, 10);
    closeSqlite(sql);

    if (pathVerif != "none")
        statistic_attack(pathOut,pathVerif);

    return 1;
}