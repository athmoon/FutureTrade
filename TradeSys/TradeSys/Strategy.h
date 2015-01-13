#ifndef __STRATEGY__
#define __STRATEGY__
#include "ConfigFile.h"
#include "DataInterface.h"
#include "Trader.h"

#define OPEN_VOL 1

#define LOSS	5
#define SLIP_POINT 5

#define MAX_TRADE_NUM 4

///Âò
#define THOST_FTDC_D_Buy '0'
///Âô
#define THOST_FTDC_D_Sell '1'

#define THOST_IGNORE	'-1'

class Strategy
{
public:
	Strategy(DataInterface* pData,CtpTraderSpi* pTrader, struct user_config* cfg);
	~Strategy();

	void StartToTrade();

	//concrete strategy

	char MA20_Open( double ma20, double loss,  double price );
	bool MA20_Close(TThostFtdcDirectionType dirType, TThostFtdcPriceType	OpenPrice, double ma20, double loss, \
		double price );

	//void Strate_LOW_HIGH(int nDay);
	//void Strate_MA_BAIS(int nDay);


	void Trade(char* IDs, TThostFtdcDirectionType dirType, int vol, double price);

	bool m_StrategyRun;

private:
	struct InstStat
	{
		SingleInsData* m_pSingle;
		int tradeNum;
	};
	DataInterface* m_pDataInterface;
	CtpTraderSpi* m_pTrader;
	struct user_config* m_cfg;

	


};


#endif