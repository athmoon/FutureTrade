#pragma once
#include "API\ThostFtdcMdApi.h"

#include "DataInterface.h"
#include "ConfigFile.h"
#include <vector>
#include <string>


class MarketPrice : public CThostFtdcMdSpi
{
public:

	MarketPrice(DataInterface * db, struct user_config* cfg);
	~MarketPrice(){}
	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo,int nRequestID, bool bIsLast);

	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	///@param nReason ����ԭ��
	///        0x1001 �����ʧ��
	///        0x1002 ����дʧ��
	///        0x2001 ����������ʱ
	///        0x2002 ��������ʧ��
	///        0x2003 �յ�������
	virtual void OnFrontDisconnected(int nReason);
		
	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
	virtual void OnHeartBeatWarning(int nTimeLapse);

	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected();
	
	///��¼������Ӧ
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///��������Ӧ��
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///ȡ����������Ӧ��
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�������֪ͨ
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);


	void setconfig(const char * addr,const char * Broker,const char * user,const char * passwd, std::vector<std::string> IDs);

	void setDataInterface(DataInterface * db);
	void StartToGetData();

	void WaitForJoin();


private:
	void ReqUserLogin();
	void SubscribeMarketData();
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);




	// ������
	int iRequestID;

	CThostFtdcMdApi* pUserApi;
	DataInterface * pDb;

	struct user_config* m_cfg;

	TThostFtdcDateType	TradingDay;

	
};
