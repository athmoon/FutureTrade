#include <iostream>
#include <windows.h>
#include "Trader.h"
#include <assert.h>

#pragma comment(lib,"Api\\thosttraderapi.lib")

#pragma warning(disable : 4099)

TThostFtdcInstrumentIDType CloseToday[32] ={"rb","ag"};


CtpTraderSpi::CtpTraderSpi(struct user_config* cfg):m_cfg(cfg)
{ 
	requestId = 0; 
	m_bUpdatePositionList = false;
};

CtpTraderSpi::~CtpTraderSpi()
{ 
	pUserApi->Release(); 
};

bool CtpTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret){
		cerr<<" ��Ӧ | "<<pRspInfo->ErrorMsg<<endl;
	}
	return ret;
}

void CtpTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}

void CtpTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr<<" ��Ӧ | ������ʱ����..." 
		<< " TimerLapse = " << nTimeLapse << endl;
}



void CtpTraderSpi::OnFrontConnected()
{
	cerr<<" ���ӽ���ǰ��...�ɹ�"<<endl;
	SetEvent(g_hEvent);
}

void CtpTraderSpi::OnFrontDisconnected(int nReason)
{
	cerr<<" ��Ӧ | �����ж�..." 
		<< " reason=" << nReason << endl;
}

void CtpTraderSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,
								TThostFtdcUserIDType	vUserId,	TThostFtdcPasswordType	vPasswd)
{

	CThostFtdcReqUserLoginField userLogin;
	memset(&userLogin, 0, sizeof(userLogin));
	strcpy(userLogin.BrokerID, vAppId); 
	strcpy(userLogin.UserID, vUserId);  
	strcpy(userLogin.Password, vPasswd);


	int ret = pUserApi->ReqUserLogin(&userLogin, requestId++);
	cerr<<" ���� | ���͵�¼..."<<((ret == 0) ? "�ɹ�" :"ʧ��") << endl;	
}

void CtpTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
								  CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if ( pRspUserLogin ) {  
		// ����Ự����	
		RspUserInfo.frontId = pRspUserLogin->FrontID;
		RspUserInfo.sessionId = pRspUserLogin->SessionID;
		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		sprintf(RspUserInfo.orderRef, "%d", ++nextOrderRef);
		cerr<<" ��Ӧ | ��¼�ɹ�...��ǰ������:"
			<<pRspUserLogin->TradingDay<<endl; 
		
		strcpy(TradeDate,pRspUserLogin->TradingDay);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_cfg->brokerID.c_str());
	strcpy(req.InvestorID, m_cfg->userID.c_str());


	int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++requestId);
	cerr<<" ���� | ���ͽ��㵥ȷ��..."<<((ret == 0)?"�ɹ�":"ʧ��")<<endl;
}


void CtpTraderSpi::OnRspSettlementInfoConfirm(
	CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	if( pSettlementInfoConfirm){
		cerr<<" ��Ӧ | ���㵥..."<<pSettlementInfoConfirm->InvestorID
			<<"...<"<<pSettlementInfoConfirm->ConfirmDate
			<<" "<<pSettlementInfoConfirm->ConfirmTime<<">...ȷ��"<<endl;
	}
	if(bIsLast) 
		SetEvent(g_hEvent);
}

void CtpTraderSpi::UserLogin()
{

	ReqUserLogin((char*)m_cfg->brokerID.c_str(),(char*)m_cfg->userID.c_str(),(char*)m_cfg->passwd.c_str());
	WaitForSingleObject(g_hEvent,INFINITE);
	ResetEvent(g_hEvent);

	Sleep(SLEEP_TIME);


	ReqSettlementInfoConfirm();
	WaitForSingleObject(g_hEvent,INFINITE);
	ResetEvent(g_hEvent);
	Sleep(SLEEP_TIME);

}


void CtpTraderSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_cfg->brokerID.c_str());
	strcpy(req.InvestorID, m_cfg->userID.c_str());
	int ret = pUserApi->ReqQryTradingAccount(&req, ++requestId);
	cerr<<" ���� | �����ʽ��ѯ..."<<((ret == 0)?"�ɹ�":"ʧ��")<<endl;

}

void CtpTraderSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	if (pTradingAccount){
		cerr<<" ��Ӧ | Ȩ��:"<<pTradingAccount->Balance
			<<" ����:"<<pTradingAccount->Available   
			<<" ��֤��:"<<pTradingAccount->CurrMargin
			<<" ƽ��ӯ��:"<<pTradingAccount->CloseProfit
			<<" �ֲ�ӯ��"<<pTradingAccount->PositionProfit
			<<" ������:"<<pTradingAccount->Commission
			<<" ���ᱣ֤��:"<<pTradingAccount->FrozenMargin
			//<<" ����������:"<<pTradingAccount->FrozenCommission 
			<< endl;    
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInstrument(&req, ++requestId);
	cerr<<" ���� | ���ͺ�Լ��ѯ..."<<((ret == 0)?"�ɹ�":"ʧ��")<<endl;
}

void CtpTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, 
									  CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 	
	if (pInstrument){
		cerr<<" ��Ӧ | ��Լ:"<<pInstrument->InstrumentID
			<<" ������:"<<pInstrument->DeliveryMonth
			<<" ��ͷ��֤����:"<<pInstrument->LongMarginRatio
			<<" ��ͷ��֤����:"<<pInstrument->ShortMarginRatio<<endl;    
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_cfg->brokerID.c_str());
	strcpy(req.InvestorID, m_cfg->userID.c_str());
	strcpy(req.InstrumentID, instId);	
	int ret = pUserApi->ReqQryInvestorPosition(&req, ++requestId);
	cerr<<" ���� | ���ͳֲֲ�ѯ..."<<((ret == 0)?"�ɹ�":"ʧ��")<<endl;
}




char CtpTraderSpi::MapDirection(char src, bool toOrig)
{
	if(toOrig)
	{
		if('b'==src||'B'==src)
		{src='0';}
		else if('s'==src||'S'==src)
		{src='1';}
	}
	else
	{
		if('0'==src)
		{src='B';}
		else if('1'==src)
		{src='S';}
	}
	return src;
}

char CtpTraderSpi::MapOffset(char src, bool toOrig)
{
	if(toOrig)
	{
		if('o'==src||'O'==src)
		{
			src='0';
		}
		else if('c'==src||'C'==src)
		{
			src='1';
		}
		else if('j'==src||'J'==src)
		{
			src='3';
		}
	}
	else
	{
		if('0'==src)
		{
			src='O';
		}
		else if('1'==src)
		{
			src='C';
		}
		else if('3'==src)
		{
			src='J';
		}
	}
	return src;
}


void CtpTraderSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField *pInvestorPosition, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	if(pInvestorPosition ){
		cerr<<" ��Ӧ | ��Լ:"<<pInvestorPosition->InstrumentID
			<<" ����:"<<MapDirection(pInvestorPosition->PosiDirection)
			<<" �ֲܳ�:"<<pInvestorPosition->Position
			<<" ���:"<<pInvestorPosition->YdPosition 
			<<" ���:"<<pInvestorPosition->TodayPosition
			<<" �ɱ�:"<<pInvestorPosition->OpenCost
			<<" �ֲ�ӯ��:"<<pInvestorPosition->PositionProfit
			<<" ��֤��:"<<pInvestorPosition->UseMargin<<endl;
	}
	if(bIsLast) 
		SetEvent(g_hEvent);	
}

int CtpTraderSpi::ReqQryInvestorPositionDetail(TThostFtdcInstrumentIDType instId)
{
	struct CThostFtdcQryInvestorPositionDetailField field;
	strcpy(field.BrokerID ,m_cfg->brokerID.c_str());
	strcpy(field.InvestorID,m_cfg->userID.c_str());
	strcpy(field.InstrumentID,"");

	if(instId)
	{
		strcpy(field.InstrumentID,instId);
	}
	int ret = pUserApi->ReqQryInvestorPositionDetail(&field, ++requestId);
	cerr<< " ���� | ���ͳֲֲ�ѯ..." <<((ret == 0)?"�ɹ�":"ʧ��") << endl;
	return ret;

}

//only call once to update the postionList

void CtpTraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPositionDetail)
	{
		cerr<< " ��Ӧ | ��ѯ�ֲ�..."
			<< "��Լ����:"<<pInvestorPositionDetail->InstrumentID
			<<" ����:"<<pInvestorPositionDetail->Direction
			<<"�ɽ����"<<pInvestorPositionDetail->TradeID
			<<"����"<<pInvestorPositionDetail->Volume
			<<"���ּ�"<<pInvestorPositionDetail->OpenPrice<<endl;

		struct PositionListItem field;
		field.Direction= pInvestorPositionDetail->Direction;
		strcpy(field.InstrumentID,pInvestorPositionDetail->InstrumentID);
		field.OpenPrice = pInvestorPositionDetail->OpenPrice;
		field.Volume = pInvestorPositionDetail->Volume;
		strcpy(field.OpenDate ,pInvestorPositionDetail->OpenDate);

		if(!m_bUpdatePositionList)
			UpdatePositionInfo(&field);

	}
	if(bIsLast) 
	{
		SetEvent(g_hEvent);	
		m_bUpdatePositionList = true;
	}
}

//OC_flag : open/close
void CtpTraderSpi::UpdatePositionInfo(struct PositionListItem *pInvestorPosition)
{
	struct PositionListItem field;
	memcpy(&field,pInvestorPosition,sizeof(struct PositionListItem));
	field.TradeNum =1;
	if(field.Volume==0)
		return;

	list<struct PositionListItem>::iterator iter = positionList.begin();
	for (; iter!=positionList.end(); iter++)
	{
		if (strcmp(iter->InstrumentID,pInvestorPosition->InstrumentID)==0 )
		{
			if(iter->Direction==pInvestorPosition->Direction)
			{
				iter->OpenPrice = (iter->OpenPrice+pInvestorPosition->OpenPrice)/2;
				iter->Volume++;
			}
			else
			{
				iter->Volume--;
				assert(iter->Volume>=0);
				if(iter->Volume==0)
					positionList.erase(iter);
				//iter->TradeNum++;
			}
			return;	
		}

	}

	positionList.push_back(field);
	m_bUpdatePositionList = false;

}


void CtpTraderSpi::ReqOrderInsert(TThostFtdcInstrumentIDType instId,
								  TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
								  TThostFtdcPriceType price,   TThostFtdcVolumeType vol, TThostFtdcPriceType deta)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, m_cfg->brokerID.c_str());  	
	strcpy(req.InvestorID, m_cfg->userID.c_str()); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, RspUserInfo.orderRef);  //��������
	int nextOrderRef = atoi(RspUserInfo.orderRef);
	sprintf(RspUserInfo.orderRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//�۸�����=�޼�	
	req.Direction = MapDirection(dir,true);  //��������	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //THOST_FTDC_OF_Open; //��Ͽ�ƽ��־:����
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //���Ͷ���ױ���־
	req.LimitPrice = price;	//�۸�
	req.VolumeTotalOriginal = vol;	///����	
	req.TimeCondition = THOST_FTDC_TC_GFD;  //��Ч������:������Ч
	req.VolumeCondition = THOST_FTDC_VC_AV; //�ɽ�������:�κ�����
	req.MinVolume = 1;	//��С�ɽ���:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //��������:����

	TThostFtdcPriceType	StopPrice =price + deta ;  //ֹ���
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;  //�Զ������־:��	
	req.UserForceClose = 0;   //�û�ǿ����־:��

	int ret = pUserApi->ReqOrderInsert(&req, ++requestId);
	cerr<<" ���� | ���ͱ���..."<<((ret == 0)?"�ɹ�":"ʧ��")<< endl;
}

void CtpTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
									CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInputOrder ){
		cerr<<"��Ӧ | �����ύ�ɹ�...��������:"<<pInputOrder->OrderRef<<endl;  
	}
	if(bIsLast) SetEvent(g_hEvent);	
}


void CtpTraderSpi::ReqOrderAction(TThostFtdcSequenceNoType orderSeq)
{
	bool found=false; unsigned int i=0;
	for(i=0;i<orderList.size();i++){
		if(orderList[i]->BrokerOrderSeq == orderSeq){ found = true; break;}
	}
	if(!found){cerr<<" ���� | ����������."<<endl; return;} 

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_cfg->brokerID.c_str());   
	strcpy(req.InvestorID, m_cfg->userID.c_str()); 
	strcpy(req.ExchangeID, orderList[i]->ExchangeID);
	strcpy(req.OrderSysID, orderList[i]->OrderSysID);
	req.ActionFlag = THOST_FTDC_AF_Delete;  

	int ret = pUserApi->ReqOrderAction(&req, ++requestId);
	cerr<< " ���� | ���ͳ���..." <<((ret == 0)?"�ɹ�":"ʧ��") << endl;
}

void CtpTraderSpi::OnRspOrderAction(
									CThostFtdcInputOrderActionField *pInputOrderAction, 
									CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction){
		cerr<< " ��Ӧ | �����ɹ�..."
			<< "������:"<<pInputOrderAction->ExchangeID
			<<" �������:"<<pInputOrderAction->OrderSysID<<endl;
	}
	if(bIsLast) SetEvent(g_hEvent);	
}

///�����ر�
void CtpTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{	
	CThostFtdcOrderField* order = new CThostFtdcOrderField();
	memcpy(order,  pOrder, sizeof(CThostFtdcOrderField));
	bool founded=false; 
	unsigned int i=0;
	for(;i<orderList.size(); i++)
	{
		if(orderList[i]->BrokerOrderSeq == order->BrokerOrderSeq)
		{
			founded=true;   
			break;
		}
	}
	if(founded) 
		orderList[i]= order;   
	else  
		orderList.push_back(order);
	cerr<<" �ر� | �������ύ...���:"<<order->BrokerOrderSeq<<endl;
	BrokerOrderSeq = order->BrokerOrderSeq;
	SetEvent(g_hEvent);	
}


void CtpTraderSpi::PrintOrders(){
	CThostFtdcOrderField* pOrder; 
	for(unsigned int i=0; i<orderList.size(); i++)
	{
		pOrder = orderList[i];
		cerr<<" ���� | ��Լ:"<<pOrder->InstrumentID
			<<" ����:"<<MapDirection(pOrder->Direction,false)
			<<" ��ƽ:"<<MapOffset(pOrder->CombOffsetFlag[0],false)
			<<" �۸�:"<<pOrder->LimitPrice
			<<" ����:"<<pOrder->VolumeTotalOriginal
			<<" ���:"<<pOrder->BrokerOrderSeq 
			<<" �������:"<<pOrder->OrderSysID
			<<" ״̬:"<<pOrder->StatusMsg<<endl;
	}
	SetEvent(g_hEvent);
}


void CtpTraderSpi::PrintTrades(){
	CThostFtdcTradeField* pTrade;
	for(unsigned int i=0; i<tradeList.size(); i++)
	{
		pTrade = tradeList[i];
		cerr<<" �ɽ� | ��Լ:"<< pTrade->InstrumentID 
			<<" ����:"<<MapDirection(pTrade->Direction,false)
			<<" ��ƽ:"<<MapOffset(pTrade->OffsetFlag,false) 
			<<" �۸�:"<<pTrade->Price
			<<" ����:"<<pTrade->Volume
			<<" �������:"<<pTrade->OrderSysID
			<<" �ɽ����:"<<pTrade->TradeID<<endl;
	}
	SetEvent(g_hEvent);
}


///�ɽ�֪ͨ
void CtpTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	CThostFtdcTradeField* trade = new CThostFtdcTradeField();
	memcpy(trade,  pTrade, sizeof(CThostFtdcTradeField));
	bool founded=false;     
	unsigned int i=0;
	for(i=0; i<tradeList.size(); i++)
	{
		if(tradeList[i]->TradeID == trade->TradeID)
		{
			founded=true;  
			break;
		}
	}
	if(founded) 
		tradeList[i] = trade;   
	else  
		tradeList.push_back(trade);

	struct PositionListItem field;
	field.Direction=trade->Direction;
	strcpy(field.InstrumentID,trade->InstrumentID);
	field.OpenPrice = trade->Price;
	field.Volume = trade->Volume;
	field.OffsetFlag = trade->OffsetFlag;

	if(m_bUpdatePositionList)
		UpdatePositionInfo(&field);

	cerr<<" �ر� | �����ѳɽ�...�ɽ����:"<<trade->TradeID<<endl;

	SetEvent(g_hEvent);
}

//not support buy and sell at the same position.
int CtpTraderSpi::LookupInstruments(TThostFtdcInstrumentIDType instId,struct PositionListItem& Inst)
{
	memset(&Inst,0,sizeof(struct PositionListItem));

	list<struct PositionListItem>::iterator iter = positionList.begin();
	for (; iter!=positionList.end(); iter++)
	{
		if (strcmp(iter->InstrumentID,instId)==0 )
		{
			Inst.Volume = iter->Volume;
			strcpy(Inst.InstrumentID, iter->InstrumentID);
			Inst.Direction = iter->Direction;
			Inst.OpenPrice = iter->OpenPrice;
			Inst.TradeNum = iter->TradeNum;
			strcpy(Inst.OpenDate,iter->OpenDate);
			return  1;
		}
	}

	return 0;

}
void CtpTraderSpi::Sleep_Wait(unsigned int time)
{
	Sleep(time);
	WaitForSingleObject(g_hEvent,INFINITE);
	ResetEvent(g_hEvent);
}

void CtpTraderSpi::StartToTrade()
{
	pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi(".\\flow_td\\");

	pUserApi->RegisterSpi(this);									// ע���¼���
	pUserApi->SubscribePublicTopic(THOST_TERT_RESUME);				// ע�ṫ����
	pUserApi->SubscribePrivateTopic(THOST_TERT_RESUME);			 // ע��˽����
	pUserApi->RegisterFront((char*)m_cfg->trade_addr.c_str());							// ע�ύ��ǰ�õ�ַ

	pUserApi->Init();

	Sleep_Wait(SLEEP_TIME);

	ReqUserLogin((char*)m_cfg->brokerID.c_str(),(char*)m_cfg->userID.c_str(),(char*)m_cfg->passwd.c_str());

	Sleep_Wait(SLEEP_TIME);

	ReqSettlementInfoConfirm();

	Sleep_Wait(SLEEP_TIME);

	ReqQryTradingAccount(); 

	Sleep_Wait(SLEEP_TIME);

	ReqQryInvestorPositionDetail("");

	Sleep_Wait(SLEEP_TIME);
	
}

bool CtpTraderSpi::IsCloseTodayInst(TThostFtdcInstrumentIDType instId)
{
	//loop the closeToday inst list
	struct PositionListItem instPos;
	LookupInstruments(instId, instPos);
	string delims="1";
	string str=instId; 
	size_t pos = str.find_first_of(delims); 

	str = str.substr(0,pos);

	if(strcpy(instPos.OpenDate,TradeDate)==0)
	{
		for (int i =0; i<sizeof(CloseToday)/sizeof(TThostFtdcInstrumentIDType); i++)
		{
			if(strcpy((char*)str.c_str(),CloseToday[i])==0)
				return true;
		}
		return false;
	}
	else
		return false;
}

void CtpTraderSpi::Buy_Open(TThostFtdcInstrumentIDType instId,TThostFtdcPriceType price,TThostFtdcVolumeType vol/* =1 */,TThostFtdcPriceType loss /* =0 */)
{
	TThostFtdcDirectionType dir = THOST_FTDC_D_Buy;
	TThostFtdcCombOffsetFlagType kpp;
	kpp[0] = THOST_FTDC_OF_Open;
	//strcpy(kpp,THOST_FTDC_OF_Open);
	ReqOrderInsert(instId,dir,kpp,price,vol,loss);
}

void CtpTraderSpi::Sell_Close(TThostFtdcInstrumentIDType instId,TThostFtdcPriceType price,TThostFtdcVolumeType vol/* =1 */,TThostFtdcPriceType loss /* =0 */)
{
	TThostFtdcDirectionType dir = THOST_FTDC_D_Sell;
	TThostFtdcCombOffsetFlagType kpp;
	kpp[0]= THOST_FTDC_OF_Close;
	
	if(IsCloseTodayInst(instId))
		kpp[0] = THOST_FTDC_OF_CloseToday;
	ReqOrderInsert(instId,dir,kpp,price,vol,loss);
}

void CtpTraderSpi::Sell_Open(TThostFtdcInstrumentIDType instId,TThostFtdcPriceType price,TThostFtdcVolumeType vol/* =1 */,TThostFtdcPriceType loss /* =0 */)
{
	TThostFtdcDirectionType dir = THOST_FTDC_D_Sell;
	TThostFtdcCombOffsetFlagType kpp;
	kpp[0]= THOST_FTDC_OF_Open;
	ReqOrderInsert(instId,dir,kpp,price,vol,loss);
}

void CtpTraderSpi::Buy_Close(TThostFtdcInstrumentIDType instId,TThostFtdcPriceType price,TThostFtdcVolumeType vol/* =1 */,TThostFtdcPriceType loss /* =0 */)
{
	TThostFtdcDirectionType dir = THOST_FTDC_D_Buy;
	TThostFtdcCombOffsetFlagType kpp;
	kpp[0]= THOST_FTDC_OF_Close;

	if(IsCloseTodayInst(instId))
		kpp[0] = THOST_FTDC_OF_CloseToday;
	ReqOrderInsert(instId,dir,kpp,price,vol,loss);
}