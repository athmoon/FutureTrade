#include <stdlib.h>
#include <stdio.h>
#include "ConfigFile.h"



config_class::config_class(char * file)
{

	//load the config file into priv_config
	FILE * fp  = fopen(file, "r");


	char buff[512];
	while(fgets(buff, 512, fp))
	{
		char delims[] = " ";
		char *result = NULL;
		struct parameter para;

		result = strtok(buff , delims );
		strcpy(para.name , result);
		result = strtok(NULL , delims );
		while( result != NULL ) 
		{
			//the last char is '\n', other char ?  remove if?
			if(result[strlen(result)-1] == '\n')
			{
				result[strlen(result)-1]  = 0;
			}
			//for(int i = 0 ; i < strlen(result); i++)
			para.value.push_back(result);
			result = strtok( NULL, delims );
		}

		priv_config.push_back(para);
	}


	//para into m_cfg
	get_usercfg(m_cfg);

}

void config_class::get_string(char * name , string & data)
{
	unsigned int i = 0;
	for(i = 0 ; i < priv_config.size(); i++)
	{
		if(!strcmp(name, priv_config[i].name))
		{
			data = priv_config[i].value[0]; 
			return;
		}
	}
}
void config_class::get_vector(char * name, vector<string> &data)
{
	unsigned int i = 0;
	for(i = 0 ; i < priv_config.size(); i++)
	{
		if(!strcmp(name, priv_config[i].name))
		{
			data = priv_config[i].value;
			return ;
		}
	}
}



void config_class::get_usercfg(user_config& cfg)
{
	get_string(FRONT_ADDR, cfg.front_addr);
	get_string(TRADE_ADDR,cfg.trade_addr);
	get_string(BROKER_ID, cfg.brokerID);
	get_string(USER_NAME, cfg.userID);
	get_string(PASSWD, cfg.passwd);
	get_vector(INSTRUMENT_ID, cfg.IDs);
}
