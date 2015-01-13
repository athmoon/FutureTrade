#include <vector>
#include <string>
#include <iostream>
#include <time.h>
using namespace std;

#include "DataInterface.h"
#include "Data_db.h"
#include "Trade_Util.h"


DataInterface::DataInterface()
{
	db = new DataDB("data_db");
}

DataInterface::~DataInterface()
{
	delete db;
}

void  DataInterface::create_tick_key(char * key, char * Instrument, int  InstrumentNum, unsigned long long start_tm)
{
	memset(key, 0x00, KEY_SIZE);
	sprintf(key, "%s_%04d_%016lld", Instrument, InstrumentNum, start_tm);

	return;
}

void  DataInterface::create_1min_key(char * key, char * Instrument, int  InstrumentNum, unsigned long long start_tm)
{
	memset(key, 0x00, KEY_SIZE);
	sprintf(key, "min1_%s_%04d_%016lld", Instrument, InstrumentNum, start_tm);

	return;
}

void  DataInterface::create_day_key(char * key, char * Instrument, int  InstrumentNum, unsigned long long start_tm)
{
	memset(key, 0x00, KEY_SIZE);
	sprintf(key, "day_%s_%04d_%016lld", Instrument, InstrumentNum, start_tm);

	return;
}

void DataInterface::GenKey(enum Enum_Key_type Key_type, char * key, char * Instrument, int  InstrumentNum, unsigned long long tm)
{
	switch(Key_type)
	{
	case TICK_Key:
		create_tick_key(key, Instrument, InstrumentNum, tm);
		break;
	case MIN_1_Key:
		create_1min_key(key, Instrument, InstrumentNum, tm);
		break;
	case DAY_Key:
		create_day_key(key, Instrument, InstrumentNum, tm);
		break;
	default:
		break;
	}		
	return ;
}

int DataInterface::end_batch()
{
	return db->put_batch();
}

int DataInterface::start_batch()
{
	return db->clear_batch();
}

int DataInterface::PutData(enum Enum_Key_type Key_type, char * Instrument, int InstrumentNum, unsigned long long tm, const struct tick_data_type & data)
{
	char key[256];

	GenKey(Key_type,key,Instrument,InstrumentNum,tm);

	return db->insert_data(key, 256, &data, sizeof(data));
}


int DataInterface::Put_Data_batch(char * key, void * data, int data_size)
{
	return db->insert_data_batch(key, 256, data, data_size);
}

int	DataInterface::Put_Tick_Data_batch(char * Instrument, int InstrumentNum, unsigned long long  tm, const struct tick_data_type &  data)
{
	char key[256];

	GenKey(TICK_Key,key,Instrument,InstrumentNum,tm);

	return db->insert_data_batch(key, 256, &data, sizeof(data));
}

int DataInterface::GetOneData(enum Enum_Key_type key_type, char * Instrument, int  InstrumentNum, unsigned long long tm ,void *data, int size)
{
	char key[256];
	string d;

	GenKey(key_type, key, Instrument, InstrumentNum,tm);

	if(db->get_data(key, 256, d)> 0)
	{
		memcpy(data, d.c_str(), size);
		return 1;
	}

	return -1;
}


int DataInterface::GetTick_range(char * Instrument, int  InstrumentNum, unsigned long long start_tm , 
								 unsigned long long end_tm, vector<struct tick_data_type > & data_out)
{
	leveldb::Iterator* it = db->get_leveldb()->NewIterator(leveldb::ReadOptions());
	if(!it)
		return 1;

	char start[KEY_SIZE];
	char end[KEY_SIZE];

	create_tick_key(start,  Instrument, InstrumentNum, start_tm);
	create_tick_key(end,  Instrument, InstrumentNum, end_tm);

	leveldb::Slice s_start((char*)start, sizeof(start));
	leveldb::Slice s_end((char*)end, sizeof(end));

	for (it->Seek(s_start); (it->Valid()) && (it->key().ToString() < end); it->Next()) 
	{
		leveldb::Slice slice = it->value(); 
		struct tick_data_type * data = (struct tick_data_type * )slice.data();

		data_out.push_back(*data);
	}

	if(!it->status().ok())
	{
		std::cout<<it->status().ToString();
	}
	assert(it->status().ok());  // Check for any errors found during the scan
	delete it;

	return 1;
}


int DataInterface::GetData_range(char * Instrument, int InstrumentNum, unsigned long long  start_tm , \
								 unsigned long long  end_tm, struct rang_data_type * data)
{
	std::vector<struct tick_data_type> data_in;
	GetTick_range(Instrument, InstrumentNum, start_tm, end_tm, data_in);

	if(data_in.size() > 0)
	{
		data->open_price = data_in[0].last_Price;
		data->close_price = data_in[data_in.size() -1].last_Price;

		data->high = data_in[0].last_Price;
		data->low = data_in[0].last_Price;
		data->volume = 0;

		for(unsigned int i = 0 ; i < data_in.size(); i++)
		{
			data->volume += data_in[i].volume;

			if(data_in[i].last_Price > data->high)
			{
				data->high = data_in[i].last_Price;
			}

			if(data_in[i].last_Price < data->low)
			{
				data->low = data_in[i].LowestPrice;
			}
		}

		return 1;
	}

	data->high = 0;

	return -1;
}



int DataInterface::GetLastDayPrice(TThostFtdcDateType	TradingDay, char * Instrument, int InstrumentNum,struct rang_data_type * data)
{
	int year=0; int month =0 ; int day=0;
	int _year=0; int _month =0 ; int _day=0;
	bool isEnd =false;
	sscanf(TradingDay, "%d:%d:%d", &year, &month, &day);

	for(int step=1; isEnd==true; step++ )
	{
		 CalculateYesterday(_year, _month, _day, step);
		 int weekDay = CaculateWeekday(_year, _month, _day);
		 if(weekDay!=0 || weekDay!=6)
			 isEnd = true;
	}

	unsigned long long tm_s = time_convert(_year, _month, _day ,9 , 5, 0, 0);
	unsigned long long tm_e = time_convert(_year, _month, _day, 15 , 0, 0, 0);

	int res =GetData_range(Instrument, InstrumentNum, tm_s, tm_e, data);

	return res;
}

//void DataInterface::SetKeyType(Enum_Key_type key_type)
//{
//	Key_type = key_type;
//	return;
//}



//////////////////////////////////////////////////////////////////////////
/*                            SingleInsData                            */
//////////////////////////////////////////////////////////////////////////

SingleInsData::SingleInsData(DataInterface * in, char * Instrument)
{
	PB_mode = 0;
	time_t tm = time(NULL);

	last_tm = (tm )*1000;

	m_DataInterface = in;
	assert(Instrument);
	strcpy(Ins, Instrument);
}

void SingleInsData::SetPBMode(unsigned long long start_tm)
{
	last_tm = start_tm;
	PB_mode = 1;
}

int SingleInsData::GetLastMinData(struct k_data_type * data)
{
	unsigned long long tm = (last_tm/(60*1000))*60*1000;
	if(min_tm != tm)
	{
		GetMinData(tm , data);
		min_tm = tm;
		return 1;
	}

	return 0;
}

//if tm is not min align?
int SingleInsData::GetMinData(unsigned long long tm, struct k_data_type * data)
{
	return m_DataInterface->GetOneData(MIN_1_Key,Ins, 0, tm, data, sizeof(struct k_data_type));
}

int SingleInsData::GetLastTickData(std::vector<struct tick_data_type > &data)
{
	unsigned long long endtm = 999999999999999ULL;
	if(PB_mode == 1)
	{
		endtm = last_tm + 600;
	}

	int ret = m_DataInterface->GetTick_range(Ins, 0, last_tm , endtm , data);
	if(data.size() > 0)
	{
		last_tm = data[data.size()-1].tm+1;
		return 1;
	}

	if(PB_mode == 1)
	{
		last_tm = endtm;
	}

	return 0;
}
