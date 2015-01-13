#include<iostream>
#include<windows.h>

#include "ConfigFile.h"
#include "DataInterface.h"
#include "Data_loader.h"
#include "MarketPrice.h"
#include "Trader.h"
#include "Strategy.h"


//git website private:  https://github.com/zhangyangang/TradeSys.git
//git website public :  https://github.com/liangam/FutureTrade.git
//author : sam zhang, allen liang



int main(int argc, char* argv[])
{
	
	struct user_config cfg;
	config_class config(CONFIG_FILE);
	config.get_usercfg(cfg);
	//SingleInsData(NULL,NULL);

	//Data_loader dl;
	//dl.ReadDayFile_QDA("sQ20140516.QDA");
	//dl.Readtick_QRT("20130104.QRT");

	DataInterface data_interface;
	MarketPrice MP_hd(&data_interface,&cfg);
	MP_hd.StartToGetData();


	CtpTraderSpi trader(&cfg);
	trader.StartToTrade();

	//strategy: one strategy one thread
	Strategy strategy(&data_interface,&trader,&cfg);
	strategy.StartToTrade();
	

	
	


	MP_hd.WaitForJoin();



	return 0;
}