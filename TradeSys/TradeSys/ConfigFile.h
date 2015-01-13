#pragma once
#include <vector>
#include <string>

using namespace std;

#pragma warning(disable : 4099)

#define FRONT_ADDR		"Front_Address"
#define TRADE_ADDR		"Trade_Address"
#define BROKER_ID		"Broker_ID"
#define USER_NAME		"User_name"
#define PASSWD			"PassWd"
#define INSTRUMENT_ID	"Instrument_ID"

#define CONFIG_FILE		"./config/test.conf"

#define SLEEP_TIME		2000

//only for test
//#define FRONT_ADDR		"tcp://180.166.0.226:28213"
//#define TRADE_ADDR		"tcp://180.166.0.226:28205"
//#define BROKER_ID		"0034"
//#define USER_NAME		""
//#define PASSWD			""
//#define INSTRUMENT_ID	"ta1409"

struct user_config
{
	string front_addr;
	string trade_addr;
	string brokerID;
	string userID;
	string passwd;
	vector<string> IDs;
};

struct parameter
{
	char name[64];
	std::vector<std::string> value;
};



class config_class
{
public:
	config_class(char * file);

	void get_string(char * name , string & data);
	void get_vector(char * name, vector<string> &data);
	void get_usercfg(struct user_config& cfg);

private:
	std::vector<struct parameter> priv_config;

	struct user_config m_cfg;
};