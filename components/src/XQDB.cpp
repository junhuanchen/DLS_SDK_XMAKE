// // #include "common.h"
// #include "XQDB.h"
// // #include "XQShellTool.h"
// #include <algorithm>
// #include <iterator>

// XQDB::XQDB(std::string DBfile, std::string ReCordDBfile) : m_DBfile(DBfile), m_ReCordDBfile(ReCordDBfile)
// {
//     m_DBfile="/usr/data/app/cfg/user.db";
//     m_ReCordDBfile="/usr/data/app/cfg/record.db";
//     printf("XQDB::(%s %s))\n", m_DBfile.c_str(), m_ReCordDBfile.c_str());
//     m_db = 0;
//     m_db1 = 0;
//     OpenDB_();
// }

// XQDB::~XQDB()
// {
//     CloseDB_();
// }

// bool XQDB::OpenDB()
// {

//     return true;
// }

// void XQDB::CloseDB()
// {
// }


// void XQDB::ReOpenDB()
// {
//     CloseDB_();
//     OpenDB_();
// }

// bool XQDB::OpenDB_()
// {
//     // printf("===============================1\n");
//     if (m_db)
//         return true;
//     XQString db_name = m_DBfile.c_str();
//     int ret = sqlite3_open(db_name.c_str(), &m_db);
//     if (SQLITE_OK != ret)
//     {
//         printf("Open %s error , ret: %d\n", db_name.c_str(), ret);
//         return false;
//     }

//     if (m_ReCordDBfile.size())
//     {

//         ret = sqlite3_open(m_ReCordDBfile.c_str(), &m_db1);
//         if (SQLITE_OK != ret)
//         {
//             printf("Open %s error , ret: %d\n", db_name.c_str(), ret);
//             return false;
//         }
//     }
//     else
//     {
//         m_db1 = m_db;
//     }
//     createTables();
//     return true;
// }

// void XQDB::CloseDB_()
// {
//     bool IsOne = (m_db == m_db1);
//     if (m_db)
//     {
//         sqlite3_close(m_db);
//         m_db = 0;
//     }
//     if (!IsOne)
//     {
//         sqlite3_close(m_db1);
//         m_db = 0;
//     }
// }

// int XQDB::createTables()
// {
//     if (NULL == m_db)
//         return -1;
//     char *errMsg = NULL;
//     int ret;
//     //用户表t_user
//     const char *tb_user_sql_create =
//         "CREATE TABLE tb_user_data ("
//         "user_id INTEGER NOT NULL UNIQUE,"         //   用户设备内的编号
//         "card_num     VARCHAR NOT NULL DEFAULT '',"            //  
//         "finger_id     INTEGER,"           //   
//         ");";
//     ret = sqlite3_exec(m_db, tb_user_sql_create, NULL, NULL, &errMsg);
//     if (SQLITE_OK != ret)
//     {
//         printf("Create user table error ret:%d, %s\n", ret, errMsg);
//         // return -2;
//     }
    
//     //进出记录表t_record
//     const char *t_record_sql_create =       
//         "CREATE TABLE t_record_data ("
//         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
//         "record_time INTEGER NOT NULL,"        
//         "acess_type INTEGER,"                         
//         "user_card     VARCHAR NOT NULL DEFAULT '',"            
//         "user_id     INTEGER,"            
//         "UNIQUE (id));";

//     ret = sqlite3_exec(m_db, t_record_sql_create, NULL, NULL, &errMsg);
//     if (SQLITE_OK != ret)
//     {
//         printf("Create t_record_data table error ret:%d, %s\n", ret, errMsg);
//     }

//     return 0;
// }

// int XQDB::GetAllItem(std::list<user_data> &p_user_datas) // bill
// {

//     p_user_datas.clear();
//     int ret = -1;
//     sqlite3_stmt *stmt;

//     const char *sql_ = "select * from tb_user_data order by user_id;";

//     ret = sqlite3_prepare(m_db, sql_, strlen(sql_), &stmt, NULL);
//     printf("ret:%d\n", ret);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetItemByUserId)\n");
//         return 0;
//     }

//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
     
//         user_data_t p_user_data;
//         p_user_data.user_id = sqlite3_column_int(stmt, 0);
//         p_user_data.card_num = sqlite3_column_int(stmt, 1);
//         p_user_data.finger_id = sqlite3_column_int(stmt, 2);
        
//         // printf("user_id:%d\n", p_user_data.user_id);

//         p_user_datas.push_back(p_user_data);

//     } // while
//     sqlite3_finalize(stmt);
// }

// int XQDB::GetUserNum()
// {
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     const char *szSQL = "select count(*)  from tb_user_data;";
//     int num = 0;
//     ret = sqlite3_prepare(m_db, szSQL, strlen(szSQL), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetUserNum)\n");
//         return 0;
//     }
//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         num = sqlite3_column_int(stmt, 0);
//     } 
//     sqlite3_finalize(stmt);
//     return num;
// }

// int XQDB::GetrecordNum()
// {
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     const char *szSQL = "select count(*) from t_record_data;";
//     int num = 0;
//     // char sql[1024];
//     // snprintf(sql, 1023, szSQL, user_id);
//     ret = sqlite3_prepare(m_db, szSQL, strlen(szSQL), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetRecordNum)\n");
//         return 0;
//     }
//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         num = sqlite3_column_int(stmt, 0);
//     } 
//     sqlite3_finalize(stmt);
//     return num;
// }

// int XQDB::GetCardNum()
// {
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     const char *szSQL = "select count(*)  from tb_user_data where card_num != \'\' ;";
//     int num = 0;
//     char sql[1024];
//     snprintf(sql, 1023, szSQL, user_id);
//     ret = sqlite3_prepare(m_db, sql, strlen(sql), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetCardNum)\n");
//         return 0;
//     }
//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         num = sqlite3_column_int(stmt, 0);
//     } 
//     sqlite3_finalize(stmt);
//     return num;
// }

// int XQDB::GetFingerNum()
// {
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     const char *szSQL = "select count(*) from tb_user_data where finger_id > 0;";
//     int num = 0;
//     char sql[1024];
//     snprintf(sql, 1023, szSQL, user_id);
//     ret = sqlite3_prepare(m_db, sql, strlen(sql), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetFingerNum)\n");
//         return 0;
//     }
//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         num = sqlite3_column_int(stmt, 0);
//     } 
//     sqlite3_finalize(stmt);
//     return num;
// }

// int XQDB::GetMaxUserId()
// {
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     const char *szSQL = "select max(user_id) from tb_user_data;";
//     int ID = 0;
//     ret = sqlite3_prepare(m_db, szSQL, strlen(szSQL), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetAcctNum)\n");
//         return 0;
//     }
//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         ID = sqlite3_column_int(stmt, 0);
//         ID++;
//     } 
//     sqlite3_finalize(stmt);
//     return ID;
// }

// bool XQDB::GetItemByUserId(int user_id, std::list<user_data> &p_user_datas)
// {
//     p_user_datas.clear();
//     int ret = -1;
//     sqlite3_stmt *stmt;

//     const char *sql_ = "select * from tb_user_data where user_id = %d;";    

//     char sql[1024];
//     snprintf(sql, 1023, sql_, user_id);
//     ret = sqlite3_prepare(m_db, sql, strlen(sql), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetItemByUserId)\n");
//         return 0;
//     }

//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         user_data_t p_user_data;
//         p_user_data.user_id = sqlite3_column_int(stmt, 0);
//         p_user_data.card_num = (const char *)sqlite3_column_text(stmt, 1);
//         p_user_data.finger_id = sqlite3_column_int(stmt, 2);
//         printf("----------------fromuserid--------------\n");
//         printf("user_id:%d\n",p_user_data.user_id);
//         printf("card_num:%s\n",p_user_data.card_num);
//         printf("finger_id:%d\n",p_user_data.finger_id);

//         p_user_datas.push_back(p_user_data);

//     } // while
//     sqlite3_finalize(stmt);
//     return 1;
// }


// int XQDB::GetItemList(int size,int offset,char * condition ,std::list<user_data> &p_user_datas)
// {
//     p_user_datas.clear();
//     char szbuf[1024];
//     //desc
//     snprintf(szbuf, 1023, "select * from tb_user_data where 1 = 1 %s order by user_id asc limit %d offset %d;", condition, size, offset);
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     ret = sqlite3_prepare(m_db, szbuf, strlen(szbuf), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetItemList)\n");
//         return 0;
//     }
//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         user_data_t p_user_data;
//         p_user_data.user_id = sqlite3_column_int(stmt, 0);
//         p_user_data.card_num = (const char *)sqlite3_column_text(stmt, 1);
//         p_user_data.finger_id = sqlite3_column_int(stmt, 2);

//         p_user_datas.push_back(p_user_data);
//     } 
//     sqlite3_finalize(stmt);
//     return 1;
// }

// int XQDB::insertData(char * data)
// {
//     if (NULL == m_db)
//         return -1;
//     char *errMsg = NULL;
//     int ret;
//     printf("insert-data:%s\n",data);

//     //const char *tb_record_sql = "DROP TABLE t_user_data;";  
//     ret = sqlite3_exec(m_db, data, NULL, NULL, &errMsg);
//     if (SQLITE_OK != ret)
//     {
//         printf("insertdata tb_user_data table error ret：%d, %s\n", ret, errMsg);
//         return -1;
//     }
//     printf("=======================\n");
//     return 0;
// }

// int XQDB::UpdateData(char * data)
// {
//     if (NULL == m_db)
//         return -1;
//     char *errMsg = NULL;
//     int ret;
//     printf("data:%s\n",data);
//     ret = sqlite3_exec(m_db, data, NULL, NULL, &errMsg);
//     if (SQLITE_OK != ret)
//     {
//         printf("updatedata tb_user_data table error ret：%d, %s\n", ret, errMsg);
//         return 0;
//     }
//     printf("=======================\n");
//     return 1;
// }

// int XQDB::DeleteAllUser()
// {
//      char szbuf[1024];
//     snprintf(szbuf, 1023, "delete from tb_user_data;");
//     if (SQLITE_OK != sqlite3_exec(m_db, szbuf, NULL, NULL, NULL))
//     {
//         printf("sqlite3_exec, error: %s\n", (const char *)szbuf);
//         return -1;
//     }
//     return 0;
// }
// int XQDB::DeleteUserByUserid(int user_id)
// {
//     char szbuf[1024];
//     snprintf(szbuf, 1023, "delete from tb_user_data where user_id = %d;", user_id);
//     if (SQLITE_OK != sqlite3_exec(m_db, szbuf, NULL, NULL, NULL))
//     {
//         printf("sqlite3_exec, error: %s\n", (const char *)szbuf);
//         return -1;
//     }
//     return 0;
// }

// int XQDB::UpdateCard(int user_id,char * card_num)
// {
//      char *errMsg = NULL;
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     char szbuf[1024];
//     snprintf(szbuf, 1023, "update tb_user_data set card_num=\'%s\' where user_id=%d;", card_num, user_id);
//     ret = sqlite3_exec(m_db, szbuf, NULL, NULL, &errMsg);
//     if (SQLITE_OK != ret)
//     {
//         printf("updatedata tb_user_data table error ret：%d, %s\n", ret, errMsg);
//         return -1;
//     }  
//     return 0;
// }

// int XQDB::GetUseridByCard(char * card,int &usr_id)
// {
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     char szbuf[1024];
//     snprintf(szbuf, 1023, "select * from tb_user_data where card_num =\'%s\';", card);
//     ret = sqlite3_prepare(m_db, szbuf, strlen(szbuf), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetUseridByCard)\n");
//         return false;
//     }

//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {

//         usr_id = sqlite3_column_int(stmt, 0);
       
//         printf("form card user_id:%d\n", usr_id);

//     } // while

//     sqlite3_finalize(stmt);
//     return true;
// }
// int XQDB::GetCardByUserId(int user_id,char *card)
// {
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     char szbuf[1024];
//     snprintf(szbuf, 1023, "select * from tb_user_data where user_id = %d;", user_id);
//     ret = sqlite3_prepare(m_db, szbuf, strlen(szbuf), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetCardByUserId)\n");
//         return false;
//     }

//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         card = (const char *)sqlite3_column_text(stmt, 1);
//         printf("%u\n", card);

//     } // while

//     sqlite3_finalize(stmt);
//     return true;
// }

// //***********************************record_table*******************************************
// int XQDB::insertRecord(char * data)
// {
//     if (NULL == m_db1)
//         return -1;
//     char *errMsg = NULL;
//     int ret;
//     printf("recorddata:%s\n",data);

//     //const char *tb_record_sql = "DROP TABLE t_user_data;";  
//     ret = sqlite3_exec(m_db, data, NULL, NULL, &errMsg);
//     if (SQLITE_OK != ret)
//     {
//         printf("insertdata t_record_data table error ret：%d, %s\n", ret, errMsg);
//         return 0;
//     }
//     return 1;
// }

// int XQDB::DeleteAllRecord()
// {
//     char szbuf[1024];
//     snprintf(szbuf, 1023, "delete from t_record_data;");
//     if (SQLITE_OK != sqlite3_exec(m_db, szbuf, NULL, NULL, NULL))
//     {
//         printf("sqlite3_exec, error: %s\n", (const char *)szbuf);
//         return -1;
//     }
//     return 0;
// }

// //--------------------------------finger_table-------------------------------
// int XQDB::UpdateFinger(int user_id,int finger_id)
// {
//     char *errMsg = NULL;
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     char szbuf[1024];
//     snprintf(szbuf, 1023, "update tb_user_data set finger_id=%d where user_id=%d;", finger_id, user_id);
//      ret = sqlite3_exec(m_db, szbuf, NULL, NULL, &errMsg);
//     if (SQLITE_OK != ret)
//     {
//         printf("updatedata tb_user_data table error ret：%d, %s\n", ret, errMsg);
//         return -1;
//     }  
//     return 0;
// }
// int XQDB::GetMaxFingerId()
// {
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     const char *szSQL = "select max(finger_id) from tb_user_data;";
//     int ID = 0;
//     ret = sqlite3_prepare(m_db, szSQL, strlen(szSQL), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetAcctNum)\n");
//         return 0;
//     }
//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         ID = sqlite3_column_int(stmt, 0);
//         ID++;
//     } 
//     sqlite3_finalize(stmt);
//     return ID;
// }
// int XQDB::GetUseridByFingerId(int finger_id,int &usr_id)
// {
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     char szbuf[1024];
//     snprintf(szbuf, 1023, "select * from tb_user_data where finger_id = %d;", finger_id);
//     ret = sqlite3_prepare(m_db, szbuf, strlen(szbuf), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetUseridByFingerId)\n");
//         return false;
//     }

//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         usr_id = sqlite3_column_int(stmt, 0);
       
//         printf("%d\n", usr_id);
//     } 

//     sqlite3_finalize(stmt);
//     return true;
// }
// int XQDB::GetFingerByUserId(int user_id,int &finger_id)
// {
//     int ret = -1;
//     sqlite3_stmt *stmt;
//     char szbuf[1024];
//     snprintf(szbuf, 1023, "select * from tb_user_data where user_id = %d;", user_id);
//     ret = sqlite3_prepare(m_db, szbuf, strlen(szbuf), &stmt, NULL);
//     if (SQLITE_OK != ret)
//     {
//         printf("sqlite3_prepare fail(GetFingeridByUserId)\n");
//         return false;
//     }

//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         int finger_id = sqlite3_column_int(stmt, 2);

//         printf("from-user-id finger_id:%d\n", finger_id);
//     } 

//     sqlite3_finalize(stmt);
//     return true;
// }

