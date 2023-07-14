/**
 * Author: Dochot.be
 * Depot: Github ()
 * Encoding: UTF-8
 * Language: C++
 * Source(s):
 **/


#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include "../resources/sha256.h"
#include "../resources/passwd-utils.hpp"
#include "hash_R.hpp"
using namespace std;

static bool LOG = false;

struct dataBase{
    string key;
    string value;
        };

/*value during execution*/
string hash_to_search = "hash_none";
string password_crack = "no-crack";
ifstream db_file;
vector<dataBase> lstV;
bool isSuccess = false;


/**
 * Open file with de dataBase
 * @param name_db : path to file
 */
ifstream openSqlite(string name_db)
{
   ifstream db_return(name_db);
    if( !db_return.is_open() )
        cout << "Can't open database" << endl;
    return db_return;
}


/**
 * Close file with the dataBase
 */
void closeSqlite()
{
    db_file.close();
    cout << "Close DataBase !" << endl;
}


/**
 * Research of last reduce  in db
 * @param reduceToCheck : reduce
 */
void checkReduce(const string& reduceToCheck)//TODO here check with map
{
    for (auto i = lstV.begin(); i != lstV.end(); ++i)
    {
        if (strcmp(i->key.c_str(), reduceToCheck.c_str()) == 0)
        {
            if (LOG)
                ::printf("%s : %s -> %s \n ", reduceToCheck.c_str(), i->key.c_str(), i->value.c_str());
            password_crack = i->value;
            isSuccess = true;
            ::printf("A value is find : %s for %s\n", password_crack.c_str(), hash_to_search.c_str());
        }
    }
}


/**
 * Transform hash to reduce, to find the last reduce
 * @param hash_ : hash to crack
 * @param id_of_reduce : actual reduce to check
 * @param number_of_reduce : maximum reduce
 * @param size_password : number of character in password
 */
void hashCheck(const string& hash_, int id_of_reduce, int number_of_reduce, int size_password)
{
    string j1 = hash_R(hash_, id_of_reduce, size_password, 16);
    if (LOG)
        ::printf("(R_%i) %s ", id_of_reduce, j1.c_str());
    string  j2;
    for (int i = id_of_reduce+1; i <= number_of_reduce; ++i)
    {
        j2 = sha256(j1);
        j1 = hash_R(j2, i, size_password, 16);
        if (LOG)
            ::printf(" -> %s -> (R_%i) %s", j2.c_str(), i, j1.c_str());
    }
    if (LOG)
        cout << endl;
    checkReduce(j1);
}


/**
 * Thread function
 * @param line : hash to crack
 * @param id_function : integer
 * @param max_reduce : maximum of reduce (integer)
 * @param sizePwd : size of password
 */
void thread_function(const string& line, int id_function, int max_reduce, int sizePwd)
{
    hashCheck(line, id_function, max_reduce, sizePwd);
}


/**
 * Read file with hash to crack
 * @param path_file : path to file
 * @param reduce : maximum reduce
 * @param size_pwd : number of character in password
 */
void read_pwdHash(const string& path_file, const string& path_ouput, int reduce, int size_pwd)
{
    ifstream file(path_file);
    ofstream outf(path_ouput);
    if (file.is_open())
    {
        string line;
        thread list_thd[reduce];
        while (getline(file, line))
        {
            hash_to_search = line;
            int j = reduce;
            int l = 0;
            while (j > 0 && !isSuccess)
            {
                list_thd[l] =  thread(thread_function,line, j, reduce, size_pwd);
                list_thd[l].join();
                --j;
                ++l;
            }
            //TODO error with this !
            //for (int i = 0; i < l; ++i)
            //    list_thd[i].detach();
            outf << password_crack << endl;
            password_crack = "no-crack";
            isSuccess = false;
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
 * Load file in memory to launch attack
 * @param sizePwd : size of reduce and password
 */
void load_file(int sizePwd)
{
    string line, pwd,hsh;
    while (getline(db_file, line))
    {
        struct dataBase d1;
        d1.key = line.substr(0, sizePwd);
        d1.value = line.substr(sizePwd+1,2*sizePwd+1);
        lstV.push_back(d1);
    }
}


/**
 * Launch a rainbow attack
 */
int main(int argc, char **argv)
{
    int reduction = 0, sizeWord = 10;
    string dataBase,pathOut,pathIn;
    string pathVerif = "none";
    if (argc >= 6)
    {
        dataBase  = argv[1];
        pathIn    = argv[2];
        pathOut   = argv[3];
        sizeWord  = ::strtol(argv[4], NULL, 10);
        reduction = ::strtol(argv[5], NULL, 10);

        if (argc == 7)
            pathVerif = argv[6];
    }
    else
    {
        cout << "ERROR : problem with arguments\n";
        cout << " -STR[path_to_db] -STR[path_hash_IN] -STR[path_decode_OUT] -INT[reduce] [OPTIONAL: -STR[path_check_decode]]" << endl;
       exit(0);
    }

    ::printf("DB->%s | FILE_IN-> %s | FILE_OUT->%s | REDUCTION->%i | SIZE_PWD->%i | FILE_CHECK->%s\n",dataBase.c_str(), pathIn.c_str(), pathOut.c_str(), reduction, sizeWord, pathVerif.c_str());

    string file_hash_input;
    string file_pwd_output;
    string file_pwd_check;

    db_file = openSqlite(dataBase);
    cout << "Load dataBase in memory !" << endl;
    load_file(sizeWord);
    cout << "Launch Rainbow attack !" << endl;
    read_pwdHash(pathIn,pathOut, reduction, sizeWord);
    closeSqlite();

    if (pathVerif != "none")
        statistic_attack(pathOut,pathVerif);

    return 1;
}