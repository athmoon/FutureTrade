
#include <iostream>
#include <Windows.h>

#include "MarketPrice.h"
#include "Trade_Util.h"

using namespace std;

#pragma warning(disable : 4996)
#pragma warning(disable : 4099)

#pragma comment(lib,"Api\\thostmduserapi.lib")

MarketPrice::MarketPrice(DataInterface * db, struct user_config* cfg)
{
	iRequestID = 0;
	m_cfg = cfg;
	pDb = db;

}

void MarketPrice::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
	cerr << "--->>> "<< __FUNCTION__ << endl;
	IsErrorRspInfo(pRspInfo);
}

void MarketPrice::OnFrontDisconnected(int nReason)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	cerr << "--->>> Reason = " << nReason << endl;
}

void MarketPrice::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void MarketPrice::OnFrontConnected()
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	///用户登录请求
	ReqUserLogin();
}

void MarketPrice::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << __FUNCTION__ << endl;
}

void MarketPrice::WaitForJoin()
{
	pUserApi->Join();
}

bool MarketPrice::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	return bResult;
}




void MarketPrice::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_cfg->brokerID.c_str());
	strcpy(req.UserID,  m_cfg->userID.c_str());
	strcpy(req.Password,  m_cfg->passwd.c_str());

	int iResult = pUserApi->ReqUserLogin(&req, ++iRequestID);
	cerr << "--->>> 发送用户登录请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void MarketPrice::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///获取当前交易日
		cerr << "--->>> 获取当前交易日 = " << pUserApi->GetTradingDay() << endl;
		strcpy(TradingDay,pUserApi->GetTradingDay());
		// 请求订阅行情
		SubscribeMarketData();	
	}
}

void MarketPrice::SubscribeMarketData()
{
	vector<string> & ppInstrumentID = m_cfg->IDs;
	int size = ppInstrumentID.size();
	char* *buff = new char*[size]; 

	for(unsigned int i = 0 ; i< ppInstrumentID.size();i++)
	{
		buff[i] = new char[ppInstrumentID[i].size()];
		strcpy(buff[i],ppInstrumentID[i].c_str());
	}

	int iResult = pUserApi->SubscribeMarketData(buff , ppInstrumentID.size());
	cerr << "--->>> 发送行情订阅请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void MarketPrice::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << __FUNCTION__ << endl;
	cerr << "ResID " << pSpecificInstrument->InstrumentID << endl;
	cerr << "Res " << pRspInfo->ErrorID << endl;
	cerr << nRequestID << endl;
	cerr << bIsLast << endl;
}

void MarketPrice::StartToGetData()
{
	pUserApi = CThostFtdcMdApi::CreateFtdcMdApi(".\\flow_md\\", true);			// 创建UserApi -- UDP mode
	pUserApi->RegisterSpi(this);						// 注册事件类
	pUserApi->RegisterFront((char*)m_cfg->front_addr.c_str());					// connect
	pUserApi->Init();

	Sleep(SLEEP_TIME);
	
}


void MarketPrice::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *data)
{
	cerr << __FUNCTION__ << endl;

	int allday = atoi(data->TradingDay);
	int hour ;
	int min;
	int sec;

	struct tick_data_type mydata;

	sscanf(data->UpdateTime, "%d:%d:%d", &hour, &min, &sec);

	//sscanf(data->TradingDay, "%4d%2d%2d", &hour, &min, &sec);

	unsigned long long tm =time_convert(allday/10000 - 1900, (allday%10000)/100, allday%100, hour, min, sec, data->UpdateMillisec);

	//printf("%lld\n", tm);

	mydata.version = 0x1;
	mydata.ask_price[0] = data->AskPrice1;
	mydata.ask_volume[0] = data->AskVolume1;
	mydata.bid_price[0] = data->BidPrice1;
	mydata.bid_volume[0] = data->BidVolume1;

	mydata.last_Price = data->LastPrice;
	mydata.SettlementPrice = data->SettlementPrice;
	mydata.turnover = data->Turnover;
	mydata.volume = data->Volume;

	mydata.average_price = data->AveragePrice;
	mydata.ClosePrice = data->ClosePrice;
	mydata.CurrDelta = data->CurrDelta;
	mydata.HighestPrice = data->HighestPrice;
	mydata.LowestPrice = data->LastPrice;
	strcpy(mydata.IDs, data->InstrumentID);
	mydata.PreDelta = data->PreDelta;
	mydata.tm = tm;

	mydata.OpenPrice = data->OpenPrice;

	pDb->PutData(TICK_Key, data->InstrumentID, 0, tm , mydata );

	//struct tick_data_type mydata1;
	//pDb->GetOneData(TICK_Key, data->InstrumentID, 0, tm ,&mydata1, sizeof(mydata1));

	cerr << data->TradingDay << " " << data->InstrumentID << " "
		<< data->ExchangeID << " " << data->ExchangeInstID << " " 
		<< data->LastPrice << " " << data->PreSettlementPrice<< " "
		<< data->PreClosePrice << " " << data->OpenInterest<< " "
		<< data->AskVolume1 << " " << data->BidVolume1<< " "<< tm<<endl;

}
