#ifndef DATAINTERFACE_H
#define DATAINTERFACE_H

#include "DataType.h"
#include "Data_db.h"

#define KEY_SIZE 256
typedef char TThostFtdcDateType[9];


enum Enum_Key_type
{
	TICK_Key,
	MIN_1_Key,
	DAY_Key
};

/* DataInterface: deal with the leveldb database  for genearl operations */

class DataInterface
{
public:
	DataInterface();
	~DataInterface();

	void GenKey(enum Enum_Key_type Key_type, char * key, char * Instrument, int  InstrumentNum, unsigned long long tm);

	int end_batch();
	int start_batch();

	int PutData(enum Enum_Key_type Key_type, char * Instrument, int InstrumentNum, unsigned long long  tm, const struct tick_data_type &  data);

	int Put_Data_batch(char * key, void * data, int data_size);
	int	Put_Tick_Data_batch(char * Instrument, int InstrumentNum, unsigned long long  tm, const struct tick_data_type &  data);

	int GetOneData(enum Enum_Key_type Key_type, char * Instrument, int  InstrumentNum, unsigned long long tm ,void *data, int size);

	//get tick data range from [start_tm, end_tm]
	int GetData_range(char * Instrument, int InstrumentNum, unsigned long long  start_tm , unsigned long long  end_tm, struct rang_data_type * data);
	int GetTick_range(char * Instrument, int InstrumentNum, unsigned long long  start_tm , unsigned long long  end_tm, std::vector<struct tick_data_type > & data);


	int GetLastDayPrice(TThostFtdcDateType	TradingDay, char * Instrument, int InstrumentNum,struct rang_data_type * data);

private:
	void  create_tick_key(char * key, char * Instrument, int  InstrumentNum, unsigned long long start_tm);
	void  create_1min_key(char * key, char * Instrument, int  InstrumentNum, unsigned long long start_tm);
	void  create_day_key(char * key, char * Instrument, int  InstrumentNum, unsigned long long start_tm);


	
	//	int Get_current_Data_start(char * Instrument , int InstrumentNum,  unsigned long long  start_tm ,  std::vector<struct data_type> & data_out);
	//	int Get_rid_Data(char * Instrument , int InstrumentNum , int rid, unsigned long long  start_tm , unsigned long long  end_tm, std::vector<struct data_type> & data);
	//	void SetKeyType(Enum_Key_type key_type);


private:
	class DataDB * db;

};




class SingleInsData
{
public:
	SingleInsData(DataInterface * in, char * Instrument);
	~SingleInsData() {};


	int GetLastMinData(struct k_data_type * data);
	int GetMinData(unsigned long long tm, struct k_data_type * data);   //general tm , not last tm

	int GetLastTickData(std::vector<struct tick_data_type > &data);


	//not use
	void SetPBMode(unsigned long long start_tm);

	char* GetInst(){return Ins; }

	//	int GetData_range(char * Instrument, int InstrumentNum, unsigned long long  start_tm , unsigned long long  end_tm, struct rang_data_type * data);
	//	int PutData(char * Instrument, int InstrumentNum, unsigned long long  tm, const struct data_type &  data);

	//	int Get_current_Data_start(char * Instrument , int InstrumentNum,  unsigned long long  start_tm ,  std::vector<struct data_type> & data_out);
	//	int Get_rid_Data(char * Instrument , int InstrumentNum , int rid, unsigned long long  start_tm , unsigned long long  end_tm, std::vector<struct data_type> & data);


private:
	char Ins[32];
	int PB_mode;
	unsigned long long  last_tm;			//ms as unit
	unsigned long long	min_tm;
	DataInterface * m_DataInterface;

};

#endif