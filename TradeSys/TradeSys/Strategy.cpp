#include <windows.h>

#include "Strategy.h"

double abs(double x) 
{ 
	if (x<0) 
		return -1*x; 
	else 
		return x;
}


Strategy::Strategy(DataInterface* pDataInterface, CtpTraderSpi* pTrader, struct user_config* cfg)
{
	assert(pDataInterface);
	assert(pTrader);
	assert(cfg);
	m_pDataInterface = pDataInterface;
	m_pTrader = pTrader;
	m_cfg = cfg;
	m_StrategyRun = true;
}


Strategy::~Strategy()
{
}

//General Trade  flow for different strategy
void Strategy::StartToTrade()
{
	//1. gen all instrument SingleInsData
	std::vector<struct InstStat> vecSingleData;

	for (vector<string>::iterator iter = m_cfg->IDs.begin(); iter!=m_cfg->IDs.end(); iter++)
	{
		SingleInsData* pSingle = new SingleInsData(m_pDataInterface, (char*)(*iter).c_str());
		struct InstStat _InstStat;
		_InstStat.m_pSingle = pSingle;
		_InstStat.tradeNum = 0;
		vecSingleData.push_back(_InstStat);
	}

	//m_pTrader->SetUpdateFlag(false);
	std::vector<struct tick_data_type > tickData;
	struct PositionListItem pos;
	while(m_StrategyRun)  // could gen thread each strategy
	{
		//2.loop all instruments
		for (std::vector<struct InstStat>::iterator iter = vecSingleData.begin(); iter!=vecSingleData.end(); iter++)
		{
			//2.1 Get the latest price o this instruments
			(*iter).m_pSingle->GetLastTickData(tickData);
			if(tickData.size())
			{
				//2.2  lookup the related instruments list
				//if(m_pTrader->GetUpdateFlag())	
				//同时支持软件开仓平仓和程序化,但不支持同时买卖一个品种
				int res = m_pTrader->LookupInstruments((*iter).m_pSingle->GetInst(),pos);
				//get all info for judge condition and trade operation
				
				double last_Price = tickData[0].last_Price;
				double ma20 = 0;

				if(res && pos.Volume>=OPEN_VOL && (*iter).tradeNum<MAX_TRADE_NUM)
				{
					//查询多空单	
					TThostFtdcDirectionType dirType = pos.Direction;
					TThostFtdcPriceType OpenPrice = pos.OpenPrice;

					//符合条件平仓,各种策略 replace the MA20_Close()
					if(MA20_Close(dirType,OpenPrice,ma20,LOSS,last_Price))
					{
						Trade((*iter).m_pSingle->GetInst(), dirType, pos.Volume, last_Price);
						(*iter).tradeNum ++;
					}

				}
				else
				{
					//符合条件开仓，各种策略replace the MA20_Open()
					char res = MA20_Open(ma20, LOSS, last_Price );
					if(res!=THOST_IGNORE)
					{
						Trade((*iter).m_pSingle->GetInst(), res, pos.Volume, last_Price);
					}

				}
			}
		}
		//m_pTrader->SetUpdateFlag(true);
	}


	// 3. release all SingleInsData
	for (std::vector<struct InstStat>::iterator iter = vecSingleData.begin(); iter!=vecSingleData.end(); iter++)
	{
		delete (*iter).m_pSingle;
	}

	vecSingleData.clear();

}

char Strategy::MA20_Open(double ma20, double loss,  double last_Price )
{
	if(abs(last_Price-ma20)<loss)
	{
		if(last_Price>ma20)
			return THOST_FTDC_D_Buy; //THOST_FTDC_D_Sell: 1   THOST_FTDC_D_Buy:0
		else
			return THOST_FTDC_D_Sell;
	}
	return (char)THOST_IGNORE;
}

bool Strategy::MA20_Close(TThostFtdcDirectionType dirType, TThostFtdcPriceType	OpenPrice, double ma20, double loss,  double price )
{
	if(dirType ==THOST_FTDC_D_Sell)
	{
		if(price>(ma20 + loss))
			return true;
	}
	else
	{
		if(price <(ma20 - loss))
			return true;
	}
	return false;
}	

void Strategy::Trade(char* IDs, TThostFtdcDirectionType dirType, int vol, double price)
{
	if(dirType ==THOST_FTDC_D_Sell)
	{
		m_pTrader->Buy_Close(IDs,price+SLIP_POINT,vol);
	}
	else if(dirType ==THOST_FTDC_D_Buy)
	{
		m_pTrader->Sell_Close(IDs,price-SLIP_POINT,vol);
	}
	else
		assert(1);
}
/*
void Strategy::Strate_MA(int nDay)
{

	
	SingleInsData SingleData(m_pDataInterface, (char*)m_cfg->IDs[0].c_str());

	
	
			//for(vector<struct tick_data_type>::const_iterator itor=tickData.begin(); itor!=tickData.end(); itor++)
			{
				double last_Price = tickData[0].last_Price;
				m_pTrader->Buy_Open((char*)m_cfg->IDs[0].c_str(),last_Price-100);
				Sleep(SLEEP_TIME);
				m_pTrader->Sell_Open((char*)m_cfg->IDs[0].c_str(),last_Price+100);
				Sleep(SLEEP_TIME);
				
			}
	}
#if 0
	float ma_20=0; 
	int res =-1;

	

	int flag[32];
	memset(flag,0,sizeof(flag)*sizeof(int));
	
	


	std::vector<struct tick_data_type > tickData;
	for (vector<SingleInsData*>::iterator iter = vecSingleData.begin(); iter!=vecSingleData.end(); iter++)
	{
		iter->->GetLastTickData(tickData);
		if(tickData.size())
		{
			float price = tickData.last_Price;

			if(!m_bUpdatePositionList)
			{
				struct PositionListItem pos;
				int res = m_pTrader->LookupInstruments(iter->Ins.c_str(),pos);
				if(res)
				{
					//平
				}
				else
				{
					//开
				}			
			}
			else
			{

			}

		}
	}
	m_bUpdatePositionList = true;
#endif

}

*/