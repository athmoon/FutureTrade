#ifndef DATA_LOADER
#define DATA_LOADER

#include "DataInterface.h"


/* 
	data source :
		ftp://datadown:88158@down.58851.net/sq/

	ref:
		http://blog.sina.com.cn/s/blog_73675a6d0100uoqh.html
*/

#define QH_DAY_PATH  "data_lib\\qhDay"


struct UnkownData
{
	union
	{
		char a[4];

		int b;
		float c;
		short e[2];
	};
};

struct header
{
	int Header;
	int type;
	int StockCount;
};

struct data_struct
{
	char Id[12];
	char Name[12];
	int DayCount;
};

struct day_struct
{
	int Tm;
	float OpenPrice;
	float HighPrice;
	float LowPrice;
	float ClosePrice;
	float Volum;
	float amount;
	int DealCount;
};


struct QRT_header
{
	int Magic;
	int Version;
	int Unkown;
	int RecoderSize;
	int Unkown2;
	int DataStart;
	int Unkown3;
	int CodeNum;
};

struct QRT_Iterm1
{
	char ID[10];
};

struct QRT_Iterm2
{
	int tick_count;
	short tick_Position[33];
	short unkown_1;
	struct UnkownData a[3];

	float SettlementPrice;
	float OpenPrice;
	float HighPrice;
	float LowPrice;//?
	float ClosePrice;
	float TdVolume;
	struct UnkownData b;
	float Position;//持仓
	struct UnkownData c;
	float change;//涨跌
	struct UnkownData d;
	float changeRation;//涨跌幅

	struct UnkownData e[28];

	//float f_data[61];
};

struct tick_data
{
	unsigned int tm;
	float price;
	float amount;//成交量
	float amountPrice;//成交额
	short buy[3];
	short sell[3];
	char AbsBuyP[3];
	char AbsSellP[3];
	char unknow[2]; 
};


class Data_loader
{
public:
	Data_loader(){prev_tm = -1;  prev =0; };
	~Data_loader(){};

	void ReadDayFile_QDA(char* file);

	void Readtick_QRT(char * filename);

	int Readtick_QM1(char * filename, int year, int mon);

private:

	void tick_data_output(FILE* fp, struct tick_data * data  , char * IDs);

	char ID_convert(char * out, char * in_ID, int year, int month);

	DataInterface * m_dataInterface;

	float prev;

	unsigned long long prev_tm;
};

#endif