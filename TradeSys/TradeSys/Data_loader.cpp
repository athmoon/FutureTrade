#include "Data_loader.h"

#include <time.h>
#include <stdio.h>


void Data_loader::ReadDayFile_QDA(char* file)
{
	struct header header;
	struct data_struct data;
	struct day_struct day;
	char filename[256];

	sprintf(filename,"%s\\%s",QH_DAY_PATH,file);
	FILE * fp = fopen(filename, "rb");
	fread(&header, sizeof(header), 1, fp);


	sprintf(filename,"%s\\%s.txt",QH_DAY_PATH,file);
	FILE *out = fopen(filename, "w");
	for(int i = 0 ; i < header.StockCount; i++)
	{
		fread(&data, sizeof(data), 1, fp);
		fprintf(out, "%s  %d\n", data.Id, data.DayCount);
		for(int j = 0 ; j < data.DayCount; j++)
		{
			fread(&day, sizeof(day), 1, fp);
			fprintf(out , "%d,%f,%f,%f,%f,%f,%f,%d \n", day.Tm, day.OpenPrice, day.HighPrice, day.LowPrice, day.ClosePrice,
				day.Volum, day.amount, day.DealCount);
		}
	}
	fclose(out);
	fclose(fp);

}


void Data_loader::Readtick_QRT(char * filename)
{
	char file[256];

	sprintf(file,"%s\\%s",QH_DAY_PATH,filename);
	FILE * fp = fopen(file, "rb");

	if(!fp)
		return ;

	//Load the src file
	unsigned long sek = fseek(fp, 0 , SEEK_END);
	unsigned long size_a=ftell(fp);
	char * dd = (char *)malloc(size_a);

	fseek(fp, 0 , SEEK_SET);
	fread(dd, size_a, 1, fp );
	fclose(fp);

	printf("%s \n", filename);

	struct QRT_header * header;
	struct QRT_Iterm1 * ID;
	struct QRT_Iterm2  Data;
	FILE *out = NULL;

	header = (struct QRT_header * )dd;

	m_dataInterface->start_batch();

	for(int i = 0 ; i  < header->CodeNum ; i++)
	{
		int offset = sizeof(struct QRT_header) + i*(sizeof(struct QRT_Iterm1)+sizeof(struct QRT_Iterm2));
		ID  = (struct QRT_Iterm1 *) (dd+ offset);
		offset += sizeof(struct QRT_Iterm1);
		memcpy( &Data, (dd+ offset), sizeof(struct QRT_Iterm2));

		int	prev = 0;
		//if(Data.tick_count < 10)
		int total = 0;

		for(int m = 0 ; m < 33; m++)
		{
			if(Data.tick_Position[m] == -1)
				break;
			struct tick_data  tData;
			for(int j = 0 ; j < header->RecoderSize; j++)
			{
				memcpy(&tData ,dd+header->DataStart + (header->RecoderSize*sizeof(struct tick_data)*Data.tick_Position[m]) + j*sizeof(tData), sizeof(tData));
				total ++;

				tick_data_output(out,& tData, ID->ID);
#if 0
				time_t iTime = tData.tm - 8*60*60;	//UTC to +8 tm		
				struct tm *t = localtime(&iTime);

				fprintf(out, "%d%02d%02d  %02d:%02d:%02d %f %f \n",t->tm_year+1900,t->tm_mon+1, t->tm_mday, t->tm_hour,t->tm_min,t->tm_sec , tData.price, tData.amount - prev);
				prev = tData.amount;
#endif

				if(total >= Data.tick_count)
					break;
			}
		}
	}

	m_dataInterface->end_batch();
}

char Data_loader::ID_convert(char * out, char * in_ID, int year, int month)
{
	char tt[32];
	int tt_count =0 ;
	int num = 0;
	memset(tt, 0x00, 32);
	memset(out, 0x00, 32);

	for(unsigned int i = 0 ; i < strlen(in_ID); i++)
	{
		if(in_ID[i] >= '0' && in_ID[i] <= '9')
		{
			tt[tt_count++] = in_ID[i];
		}
		else
		{
			out[i] = in_ID[i];
		}
	}
	if(tt_count > 0)
	{
		num = atoi(tt);

		if(tt_count == 2)
		{
			if(num <= month)
			{
				year++;
			}
			num += year*100;
		}

		sprintf(out, "%s%d", out, num);
	}
	return 1;
}

void Data_loader::tick_data_output(FILE* fp, struct tick_data * data  , char * IDs)
{
	unsigned  long long iTime = data->tm - 8*60*60;	//UTC to +8 tm
	time_t tt= data->tm - 8*60*60;
	struct tm *t = localtime(&tt);
	struct tick_data_type mydata;

	mydata.version = 0x2;
	mydata.ask_price[0] = 0;
	mydata.ask_volume[0] = 0;
	mydata.bid_price[0] = 0;
	mydata.bid_volume[0] = 0;

	mydata.last_Price = data->price;
	//mydata.SettlementPrice = data->SettlementPrice;
	//mydata.turnover = data->Turnover;
	mydata.volume =(int) (data->amount -prev);

	//mydata.average_price = data->AveragePrice;
	//mydata.ClosePrice = data->ClosePrice;
	//mydata.CurrDelta = data->CurrDelta;
	//mydata.HighestPrice = data->HighestPrice;
	//mydata.LowestPrice = data->LastPrice;
	//strcpy(mydata.IDs, IDs);

	ID_convert(mydata.IDs , IDs, t->tm_year - 100, t->tm_mon+1);
	//mydata.PreDelta = data->PreDelta;
	mydata.tm = iTime;


	static int counte = 0;

	if(iTime == prev_tm)
	{
		mydata.tm = iTime*1000 + 500*counte;
		if(counte == 2)
		{
			mydata.tm -= 200;
		}
		counte++;
	}
	else
	{
		mydata.tm = iTime*1000;
		counte = 0;
	}

	prev_tm = iTime;

	//mydata.OpenPrice = data->OpenPrice;


	m_dataInterface->Put_Tick_Data_batch(mydata.IDs, 0, mydata.tm , mydata );
	//fprintf(fp, "%d%02d%02d , %02d:%02d:%02d ,%f ,%f , %f\n",t->tm_year+1900,t->tm_mon+1, t->tm_mday, t->tm_hour,t->tm_min,t->tm_sec , data->price, data->amount, data->amount -prev);	
	prev = data->amount;
}


int Data_loader::Readtick_QM1(char * filename, int year, int mon)
{
	FILE * fp = fopen(filename, "rb");

	if(!fp)
		return 1;

	printf("%s \n", filename);

	struct FileHeaderType
	{
		struct UnkownData U[2];
		int ItemCount;
	}header;


	struct ItemHeaderType
	{
		char Name1[8];//ag01
		struct UnkownData N1;
		char Name2[12];//by1501
		//struct UnkownData header[64];
		int min_size;
	}item_header;


	fread(&header, 1, sizeof(header), fp);


	m_dataInterface->start_batch();

	for(int i = 0 ; i <header.ItemCount ; i++ )
	{
		fread(&item_header, 1, sizeof(item_header), fp);

		struct MinDataType
		{
			int tm;
			float open;
			float high;
			float low;
			float close;
			//struct UnkownData N2;
			float volume;
			float holding;
			int tt;
		}min_data;

		char ID[32];

		memmove(item_header.Name1, item_header.Name1+2, 10);

		ID_convert(ID, item_header.Name1, year - 2000, mon);

		int PutFlag = 0;

		if(!strcmp(ID, "agMI"))
		{
			PutFlag = 1;
		}

		//char output_file[256];
		//sprintf(output_file, "%s.txt",ID);
		//FILE * out = fopen(output_file, "w");

		int count = 0;
		for(int i = 0 ; i < item_header.min_size; i++)
		{
			fread(&min_data, 1, sizeof(min_data), fp);

			time_t tt= min_data.tm - 8*60*60;
			struct tm *t = localtime(&tt);

			k_data_type one_data;
			memset(&one_data, 0x00, sizeof(one_data));

			one_data.tm = tt;
			one_data.high = min_data.high;
			one_data.low = min_data.low;
			one_data.open_price = min_data.open;
			one_data.close_price = min_data.close;
			one_data.volume = (int)min_data.volume;
			one_data.holding = (int)min_data.holding;



			char key[256];

			memset(key, 0x00, sizeof(key));
			m_dataInterface->GenKey(MIN_1_Key,key, ID, 0, one_data.tm*1000);

			if(PutFlag)
				m_dataInterface->Put_Data_batch( key, &one_data , sizeof(one_data));

			//fprintf(out, "%d%02d%02d  %02d:%02d:%02d %f %f %f %f %f %f %d\n" ,t->tm_year+1900,t->tm_mon+1, t->tm_mday, t->tm_hour,t->tm_min,t->tm_sec,
			//min_data.D1, min_data.D2, min_data.D3, min_data.D4 , min_data.volume1, min_data.volume, min_data.tt);
			//printf("%d%02d%02d  %02d:%02d:%02d\n" ,t->tm_year+1900,t->tm_mon+1, t->tm_mday, t->tm_hour,t->tm_min,t->tm_sec);	
			count++;
		}

		//fclose(out);
	}

	m_dataInterface->end_batch();



	return 1;
}