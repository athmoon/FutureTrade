#ifndef __TRADER__
#define __TRADER__

#include <vector>
#include <list>


#include "api/ThostFtdcTraderApi.h"
#include "ConfigFile.h"


using namespace std;


typedef void *	HANDLE;


struct PositionListItem
{
	TThostFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TThostFtdcDirectionType	Direction;
	///数量
	TThostFtdcVolumeType	Volume;
	///开仓价
	TThostFtdcPriceType	OpenPrice;

	///开平标志
	TThostFtdcOffsetFlagType	OffsetFlag;

	//TThostFtdcDateType	TradingDay;		//for 策略

	TThostFtdcDateType OpenDate;   //for 平今

	//交易次数
	int TradeNum;
};


class CtpTraderSpi : public CThostFtdcTraderSpi
{

public:
	CtpTraderSpi(struct user_config* cfg);
	~CtpTraderSpi();


	void UserLogin();

	void PrintOrders();
	void PrintTrades();

	int LookupInstruments(TThostFtdcInstrumentIDType instId,struct PositionListItem& pos);

	void StartToTrade();

	//buy open
	void Buy_Open(TThostFtdcInstrumentIDType instId,TThostFtdcPriceType price,TThostFtdcVolumeType vol=1,TThostFtdcPriceType loss =0);
	void Sell_Close(TThostFtdcInstrumentIDType instId,TThostFtdcPriceType price,TThostFtdcVolumeType vol=1,TThostFtdcPriceType loss =0);

	//sell open
	void Sell_Open(TThostFtdcInstrumentIDType instId,TThostFtdcPriceType price,TThostFtdcVolumeType vol=1,TThostFtdcPriceType loss =0);
	void Buy_Close(TThostFtdcInstrumentIDType instId,TThostFtdcPriceType price,TThostFtdcVolumeType vol=1,TThostFtdcPriceType loss =0);



private:

	/***************************All request and response interface************************/

	virtual void OnFrontConnected();
	virtual void OnFrontDisconnected(int nReason);

	void ReqUserLogin(TThostFtdcBrokerIDType	appId,  TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd);
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, \
		int nRequestID, bool bIsLast);

	void ReqSettlementInfoConfirm();
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,\
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	void ReqQryTradingAccount();
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, \
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	void ReqQryInstrument(TThostFtdcInstrumentIDType instId);
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, \
		int nRequestID, bool bIsLast);


	void ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId);
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, \
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	int ReqQryInvestorPositionDetail(TThostFtdcInstrumentIDType instId);
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, \
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


	void ReqOrderInsert(TThostFtdcInstrumentIDType instId, \
		TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp, \
		TThostFtdcPriceType price,   TThostFtdcVolumeType vol, TThostFtdcPriceType deta);
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo,\
		int nRequestID, bool bIsLast);

	///报单操作请求
	//撤单
	void ReqOrderAction(TThostFtdcSequenceNoType orderSeq);
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, \
		int nRequestID, bool bIsLast);


	///报单通知
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);


private:
	struct{
		int frontId;
		int sessionId;
		char orderRef[13];
	}RspUserInfo;

	char MapDirection(char src, bool toOrig=true);
	char CtpTraderSpi::MapOffset(char src, bool toOrig=true);

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	bool	IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

	virtual void OnHeartBeatWarning(int nTimeLapse);

	void UpdatePositionInfo(struct PositionListItem *pInvestorPosition);

	void Sleep_Wait(unsigned int time);

	bool IsCloseTodayInst(TThostFtdcInstrumentIDType instId);

	void SetUpdateFlag(bool value) { m_bUpdatePositionList = value; }
	bool GetUpdateFlag() { return m_bUpdatePositionList; }

private:
	struct user_config* m_cfg;
	int requestId;
	HANDLE g_hEvent;
	CThostFtdcTraderApi* pUserApi;
	bool m_bUpdatePositionList;  

	int BrokerOrderSeq;

	///交易日期
	TThostFtdcTradeDateType	TradeDate;

	vector<CThostFtdcOrderField*> orderList;
	vector<CThostFtdcTradeField*> tradeList;
	list<struct PositionListItem> positionList;  //update postion when trade
};




#endif