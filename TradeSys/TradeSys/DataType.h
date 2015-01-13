#ifndef DATA_TYPE_H
#define DATA_TYPE_H

struct day_data_type
{

};

struct rang_data_type
{
	double high;
	double low;
	double open_price;
	double close_price;
	int volume;

};

struct k_data_type
{
	unsigned long long tm;
	double high;
	double low;
	double open_price;
	double close_price;
	int volume;
	int holding;
	unsigned int ver;
	char rev[8];
};


struct tick_data_type
{
	unsigned long long tm;
	double last_Price;//current price
	double turnover;//should be price*volume

	unsigned int version; //the version of this data type

	int volume; 

	int open_interest;//hold in hand

	//for history data ,we may just have 1
	double ask_price[5];
	int ask_volume[5];
	double bid_price[5];
	int bid_volume[5];

	double average_price;//of today

	//these data , we just save now
	///今开盘
	double	OpenPrice;
	///最高价
	double	HighestPrice;
	///最低价
	double	LowestPrice;
	///今收盘
	double	ClosePrice;
	///本次结算价
	double	SettlementPrice;
	///涨停板价
	double	UpperLimitPrice;
	///跌停板价
	double	LowerLimitPrice;
	///昨虚实度
	double	PreDelta;
	///今虚实度
	double	CurrDelta;

	char IDs[32];
};


#endif