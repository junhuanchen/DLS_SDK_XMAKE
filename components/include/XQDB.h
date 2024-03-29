#ifndef __XQ_DB_H
#define __XQ_DB_H

#include "sqlite3.h"
#include <list>
#include <string>
#include "XQDB.h"

typedef struct _user_data //doorphone t_card
{
    int user_id;
    std::string card_num;
    int finger_id;
}user_data;


typedef struct _record_data //doorphone t_record
{
    int id;
    int acess_type;
    std::string user_card;
    int record_time;
    int user_id;
}record_data;

class XQDB
{
public:
    // XQDB(std::string DBfile, std::string RecordDBfile = "");
    XQDB();
    virtual ~XQDB();

private:
    sqlite3 *m_db;
    sqlite3 *m_db1;
    std::string m_DBfile;
    std::string m_ReCordDBfile;

private:
    int createTables();    

public:
    bool OpenDB_();
    void CloseDB_();
public:
    bool OpenDB();
    void CloseDB();

    //*******************yefiot********************
private:
    int dropTables();

public:
    int GetAllItem(std::list<user_data> &p_user_datas);   
    int GetUserNum();
    int GetrecordNum();
    int GetCardNum();
    int GetFingerNum();
    //--------------user_table---------------
    bool GetItemByUserId(int user_id, std::list<user_data> &p_user_datas);
    bool GetItemByFingerId(int finger_id, std::list<user_data> &p_user_datas);
    int GetMaxUserId();   
    int insertData(char * data);
    int UpdateData(char * data);
    int DeleteAllUser();
    int DeleteUserByUserid(int user_id);
    int GetItemList(int size,int offset,char * condition,std::list<user_data> &p_user_datas);

    int UpdateCard(int user_id,char* card_num);
    int GetUseridByCard(char* card,int &usr_id);
    int GetCardByUserId(int user_id,const char *card);

    int UpdateFinger(int user_id,int finger_id);
    int GetMaxFingerId();
    int GetUseridByFingerId(int finger_id,int &usr_id);
    int GetFingerByUserId(int user_id,int &finger_id);
    //-----------record_table----------------
    int insertRecord(char * data);
    int DeleteAllRecord();

    void ReOpenDB();
};

#endif



