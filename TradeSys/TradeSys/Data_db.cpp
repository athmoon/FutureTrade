#include "Data_db.h"


#ifdef RELEASE
#pragma comment(lib,"leveldb\\lib\\Release\\leveldb.lib")
#else
#pragma comment(lib,"leveldb\\lib\\Debug\\leveldb_d.lib")
#endif


//reference: http://duanple.blog.163.com/blog/static/70971767201171705113636/
//http://leveldb.googlecode.com/git-history/40768657bc8ec3ded60712eeeab7c25b1b07deca/doc/index.html

DataDB::DataDB(const char* name)
{
	leveldb::Options options;
	options.create_if_missing = true;
	options.max_open_files = 64;
	options.compression = leveldb::kSnappyCompression;
	options.block_cache = leveldb::NewLRUCache(16 * 1048576); 
	options.write_buffer_size = 100000000; //60 * 1048576;

	//leveldb_cache_t* cache;
	//options = leveldb_options_create();
	//leveldb_options_set_cache(options, cache);
	//leveldb_options_set_compression(options, leveldb_no_compression);


	leveldb::Status status = leveldb::DB::Open(options, name, &db);
	assert(status.ok());

}

int  DataDB::clear_batch()
{
	batch.Clear();

	return 1;
}

int  DataDB::put_batch()
{
	leveldb::Status s = db->Write(leveldb::WriteOptions(), &batch);

	return CHECK_STATUS(s,"batch put error!!!");

	/*if (!s.ok()) 
	{
		printf("%s\n",  s.ToString().c_str());
		printf("batch put error!!!\n");
	}*/
	//std::cerr << s.ToString() << std::endl;

	//return 1;
}


int DataDB::insert_data(const void * key, int key_size, const void * data, int size)
{
	leveldb::Status status;

	if(NULL == data || size < 0)
		return -1;


	leveldb::Slice s_key((char*)key, key_size);
	leveldb::Slice value((char*)data, size);

	status = db->Put(leveldb::WriteOptions(), s_key, value);

	return CHECK_STATUS(status,"add error!");
	

}

int DataDB::insert_data_batch(const void * key, int key_size, const void * data, int size)
{
	leveldb::Status status;

	if(NULL == data || size < 0)
		return -1;


	leveldb::Slice s_key((char*)key, key_size);
	leveldb::Slice value((char*)data, size);

	batch.Put(s_key, value);


#if 0
	if(status.ok())
		return 1;
	else
	{
		//	LOG(ERROR)<<status.ToString();
		printf("add error\n");
		return 0;
	}
#endif
	return 0;
}


int DataDB::del_data(const void * key_in, int key_size) 
{
	leveldb::Slice key((char*)key_in,  key_size);
	leveldb::Status status;

	printf("%x %d\n", (int)(*(int *)key_in) , key_size);

	db->Delete(leveldb::WriteOptions(), key);

	return CHECK_STATUS(status,"del error");

	//if(status.ok())
	//{
	//	return 1;
	//}
	//else
	//{
	//	//LOG(ERROR)<<status.ToString();
	//	printf("del error\n");
	//	return 0;
	//}

}


int DataDB::get_data(const void * key, int key_size, std::string & data)
{
	leveldb::Status status;

	leveldb::Slice s_key((char*)key, key_size);

	status = db->Get(leveldb::ReadOptions(), s_key, &data);

	return CHECK_STATUS(status);

	//if(status.ok())
	//	return 1;
	//else
	//{
	//	//LOG(ERROR)<<status.ToString();
	//	//printf("%s\n", status.ToString().c_str());
	//	return 0;
	//}

}


int DataDB::loop_db(int (*pFun)(void *ptr, void * db_data, int length), void * p)
{
	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
	if(!it)
		return 1;
	for (it->SeekToFirst(); it->Valid(); it->Next()) 
	{
		leveldb::Slice slice = it->value(); 
		void * data = (void *)slice.data();
		int len = slice.size();


		pFun(p, data, len);

	}
	assert(it->status().ok());  // Check for any errors found during the scan
	delete it;

	return 1;
}


int DataDB::CHECK_STATUS(leveldb::Status &status, char* str)
{
	if(status.ok())
		return 1;
	else
	{
		printf("%s \n", status.ToString().c_str());
		if(str)
			printf("%s\n",str);
		return 0;
	}
}