#ifndef DATA_DB
#define DATA_DB

#include <string>
#include <leveldb.h>


/* supply the lowest level read/write for leveldb */

class DataDB
{
public:
	DataDB(const char * file);

	/* put(Key,value) */

	int insert_data(const void * key , int key_size, const void * data , int data_size); 
	int insert_data_batch(const void * key, int key_size, const void * data, int size);

	int del_data(const void * key, int key_size);
	int get_data(const void * key, int key_size, std::string & data);


	int  clear_batch();
	int  put_batch();

	leveldb::DB* get_leveldb() { return db; }


	//not use now
	int loop_db(int (*pFun)(void *ptr, void * db_data, int length), void * p);


private:

	leveldb::DB* db;

	leveldb::WriteBatch batch;

	int CHECK_STATUS(leveldb::Status &status,char* str=NULL);
};


#endif



