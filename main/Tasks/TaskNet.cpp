#include "TaskNet.hpp"
#include <string.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "netdb.h"

#include "esp_log.h"

const char AP_SSID[] = "Chrenometer";
const char AP_PASSWORD[] = "elpsykongroo";

const char DATETIME_KEY[] = "dt";
const char SSID_KEY[] = "ssid";
const char PASSWORD_KEY[] = "passwd";

const u8 DAYS_IN_MONTH[] = {31,28,31,30,31,30,31,31,30,31,30,31};

const char NTP_SERVER_URL[] = "time.google.com";
const int BUF_SIZE = 256;
const int NTP_LI = 0;
const int NTP_VN = 4;
const int NTP_MODE = 3;
const int NTP_STRATUM = 16;
const int NTP_POLL = 10;
const int NTP_PREC = 0;

TaskNet* taskNet = nullptr;

const u8* getIndexPageStart();
const u8* getIndexPageEnd();


void TaskNet::Init(EventGroupHandle_t* eventGroup)
{
	mainEventGroup = eventGroup;

	taskNet = this;

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	accessPointIF = esp_netif_create_default_wifi_ap();
	stationIF = esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t wifiEvents;
	esp_event_handler_instance_t getIpEvents;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
										ESP_EVENT_ANY_ID,
										&wifiEventHandler,
										&server,
										&wifiEvents));



	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
	                                    IP_EVENT_STA_GOT_IP,
	                                    &wifiEventHandler,
	                                    nullptr,
	                                    &getIpEvents));
	wifi_config_t APConfig = {};
	memcpy(APConfig.ap.ssid, AP_SSID, strlen(AP_SSID));
	memcpy(APConfig.ap.password, AP_PASSWORD, strlen(AP_PASSWORD));
	APConfig.ap.ssid_len = strlen((char*)APConfig.ap.ssid);
	APConfig.ap.channel = 4;
	APConfig.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	APConfig.ap.max_connection = 4;

	wifi_config_t SConfig = {};
	strcpy((char*)SConfig.sta.ssid, ssid);
	strcpy((char*)SConfig.sta.password, password);
	SConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
	SConfig.sta.pmf_cfg = {
		true,
		false
	};

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &APConfig));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &SConfig));
	ESP_ERROR_CHECK(esp_wifi_start());
}

void TaskNet::SetTime(struct tm& dt)
{
	dateTime = dt;
}

tm TaskNet::GetTime()
{
	return dateTime;
}

char* TaskNet::GetPointerSsid()
{
	return ssid;
}

char* TaskNet::GetPointerPassword()
{
	return password;
}

void TaskNet::SetMainEvent(unsigned event)
{
	xEventGroupSetBits(*mainEventGroup, event);
}

esp_err_t TaskNet::NtpSync(tm& dt)
{
	hostent* info = gethostbyname(NTP_SERVER_URL);
	if(info->h_addr_list==nullptr)
	{
		return ESP_ERR_NOT_FOUND;
	}
	sockaddr_in addr;
	addr.sin_addr.s_addr = static_cast<u32>(info->h_addr_list[0][0])|(static_cast<u32>(info->h_addr_list[0][1])<<8)|
			(static_cast<u32>(info->h_addr_list[0][2])<<16)|(static_cast<u32>(info->h_addr_list[0][3])<<24);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(123);

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(sock<0)
	{
		ESP_LOGI("NET", "Failed to create socket");
		return ESP_FAIL;
	}

	timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

	u8 buf[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);
	buf[0] = (NTP_LI<<5)|(NTP_VN<<3)|NTP_MODE;
	buf[1] = NTP_STRATUM;
	buf[2] = NTP_POLL;
	buf[3] = NTP_PREC;
	strcat((char*)buf+12, "REFR");
	//ts of last sync
	//current ts
	u64 s = date2sec(dt);
	buf[24] = (s>>24)&0xff;
	buf[25] = (s>>16)&0xff;
	buf[26] = (s>>8)&0xff;
	buf[27] = s&0xff;

	int res = sendto(sock, (void*)buf, 48, 0, (sockaddr*)&addr, sizeof(addr));
	if(res<0)
	{
		ESP_LOGI("NET", "Failed to sync");
		return ESP_FAIL;
	}
	socklen_t asize = sizeof(addr);
	res = recvfrom(sock, buf, BUF_SIZE, 0, (sockaddr*)&addr, &asize);
	if(res<40)
	{
		ESP_LOGI("NET", "Failed to sync");
		return ESP_FAIL;
	}
	s = static_cast<u32>(buf[43])+(static_cast<u32>(buf[42])<<8)+(static_cast<u32>(buf[41])<<16)+(static_cast<u32>(buf[40])<<24);
	dt = sec2date(s);
	if(!isTimeValid(dt))
	{
		ESP_LOGI("NET", "Broken data");
		return ESP_FAIL;
	}
	return ESP_OK;
}

void TaskNet::wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_id == WIFI_EVENT_AP_START)
	{
	   	TaskNet::startWebServer((httpd_handle_t*)arg);
	}
	else if (event_id == WIFI_EVENT_AP_STOP)
	{
		TaskNet::stopWebServer((httpd_handle_t*)arg);
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		esp_wifi_connect();
	}
	else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
	{
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		ESP_LOGI("NET", "got ip");
	}
	else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START)
	{
	}
}

void decodeURL(char* str)
{
	int i=0;
	int offset=0;
	u8 l, h;
	while(str[i+offset]!='\0')
	{
		if(str[i+offset]=='%')
		{
			h=str[i+offset+1];
			h = h>='A' && h<='F' ? h-'A'+10 : (h>='a' && h<='f' ? h-'a'+10 : h-'0');
			l=str[i+offset+2];
			l = l>='A' && l<='F' ? l-'A'+10 : (l>='a' && l<='f' ? l-'a'+10 : l-'0');
			str[i] = (h<<4) | l;
			offset+=2;
		}
		else if(str[i+offset]=='+')
		{
			str[i]=' ';
		}
		else
		{
			str[i]=str[i+offset];
		}
		i++;
	}
	str[i]='\0';
}

static const httpd_uri_t indexGetD = {
	"/",
	HTTP_GET,
	TaskNet::IndexGet,
	nullptr
};

esp_err_t TaskNet::IndexGet(httpd_req_t* req)
{
	httpd_resp_send(req, (char *)getIndexPageStart(), getIndexPageEnd()-getIndexPageStart());
	return ESP_OK;
}

static const httpd_uri_t dateTimePostD = {
	"/datetime",
	HTTP_POST,
	TaskNet::DateTimePost,
	nullptr
};

esp_err_t TaskNet::DateTimePost(httpd_req_t* req)
{
	ESP_LOGI("NET", "datetime");
	char request[64];
	if(req->content_len>64)
	{
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Very long data");
		return ESP_FAIL;
	}
	int ret = httpd_req_recv(req, request, req->content_len);
	if (ret<=0)
	{
		if(ret == HTTPD_SOCK_ERR_TIMEOUT)
		{
			httpd_resp_send_408(req);
		}
		return ESP_FAIL;
	}

	request[req->content_len] = '\0';
	decodeURL(request);
	if(strstr(request, DATETIME_KEY) == nullptr)
	{
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Data is missing");
		return ESP_FAIL;
	}

	tm dt;
	char* begin = request+1+strlen(DATETIME_KEY);
	dt.tm_year = atoi(begin);
	begin = strchr(begin, '-')+1;
	dt.tm_mon = atoi(begin);
	begin = strchr(begin, '-')+1;
	dt.tm_mday = atoi(begin);
	begin = strchr(begin, 'T')+1;
	dt.tm_hour = atoi(begin);
	begin = strchr(begin, ':')+1;
	dt.tm_min = atoi(begin);
	dt.tm_sec = 0;
	taskNet->SetTime(dt);

	taskNet->SetMainEvent(MAIN_SET_TIME);

	httpd_resp_send(req, "Ok", HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

static const httpd_uri_t dateTimeGetD = {
	"/datetime",
	HTTP_GET,
	TaskNet::DateTimeGet,
	nullptr
};

esp_err_t TaskNet::DateTimeGet(httpd_req_t* req)
{
	char buf[64];
	if(httpd_req_get_url_query_len(req)>64)
	{
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Very long data");
		return ESP_FAIL;
	}

	if(httpd_req_get_url_query_str(req, buf, 64)!=ESP_OK)
	{
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Corrupted data");
		return ESP_FAIL;
	}
	char cmd[16] = {0};
	if(httpd_query_key_value(buf, "cmd", cmd, 16)!=ESP_OK)
	{
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Wrong data");
		return ESP_FAIL;
	}

	if(strcmp(cmd, "req")==0)
	{
		taskNet->SetMainEvent(MAIN_SET_TIME);
		httpd_resp_send(req, "Ok", HTTPD_RESP_USE_STRLEN);
	}
	else if(strcmp(cmd, "get")==0)
	{
		tm dt = taskNet->GetTime();
		sprintf(buf, "{\"datetime\":\"%d-%d-%d %d:%d:%d\"}", dt.tm_year+1900, dt.tm_mon+1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
		//sprintf(buf, "{\"datetime\":\"1234-5-6 7:8:9\"}");
		httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
	}
	else
	{
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Wrong command");
		return ESP_FAIL;
	}
	return ESP_OK;
}

static const httpd_uri_t wifiCredPostD = {
	"/wifi",
	HTTP_POST,
	TaskNet::WifiCredPost,
	nullptr
};

esp_err_t TaskNet::WifiCredPost(httpd_req_t* req)
{
	char request[128];
	if(req->content_len>128)
	{
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Very long data");
		return ESP_FAIL;
	}
	int ret = httpd_req_recv(req, request, req->content_len);
	if (ret<=0)
	{
		if(ret == HTTPD_SOCK_ERR_TIMEOUT)
		{
			httpd_resp_send_408(req);
		}
		return ESP_FAIL;
	}

	request[req->content_len] = '\0';
	decodeURL(request);
	if(strstr(request, SSID_KEY) == nullptr ||
		strstr(request, PASSWORD_KEY) == nullptr)
	{
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Data is missing");
		return ESP_FAIL;
	}

	char* ssid = request+1+strlen(SSID_KEY);
	char* passwd = strchr(ssid, '&');
	*passwd = '\0';
	passwd += 1+strlen(PASSWORD_KEY)+1;
	strcpy(taskNet->GetPointerSsid(), ssid);
	strcpy(taskNet->GetPointerPassword(), passwd);

	taskNet->SetMainEvent(MAIN_WRITE_WIFI_CRED);

	httpd_resp_send(req, "Ok", HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

static const httpd_uri_t syncNowGetD = {
	"/sync",
	HTTP_GET,
	TaskNet::SyncNowGet,
	nullptr
};

esp_err_t TaskNet::SyncNowGet(httpd_req_t* req)
{
	taskNet->SetMainEvent(MAIN_NTP_SYNC);
	httpd_resp_send(req, "Ok", HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

void TaskNet::startWebServer(httpd_handle_t* server)
{
	if (*server != nullptr)
	{
		return;
	}

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.lru_purge_enable = true;
	if(httpd_start(server, &config) == ESP_OK)
	{
	   	httpd_register_uri_handler(*server, &indexGetD);
	   	httpd_register_uri_handler(*server, &dateTimePostD);
	   	httpd_register_uri_handler(*server, &dateTimeGetD);
	   	httpd_register_uri_handler(*server, &wifiCredPostD);
	   	httpd_register_uri_handler(*server, &syncNowGetD);
	}
}

void TaskNet::stopWebServer(httpd_handle_t* server)
{
	if(*server != nullptr)
	{
		httpd_stop(*server);
		*server = nullptr;
	}
}

u64 TaskNet::date2sec(const tm& dt)
{
	u64 s = dt.tm_sec+dt.tm_min*60+dt.tm_hour*3600;

	int days = dt.tm_mday;
	int years = dt.tm_year;
	for(int m=0; m<dt.tm_mon; m++)
	{
		days += DAYS_IN_MONTH[m];
	}

	int leap;
	if(years<100)
	{
		leap = years/4;
		leap -= years/100;
		leap += years/400;
		if(years!=0 && (years%400==0 || (years%4==0 && years%100!=0)))
		{
			if (dt.tm_mon<=1)
				leap--;
		}
	}
	else
	{
		years -= 100;
		days += 36524;
		leap = years/4 + 1;
		leap -= years/100;
		leap += years/400;
		if(years%400==0 || (years%4==0 && years%100!=0))
		{
			if (dt.tm_mon<=1)
				leap--;
		}
	}

	days += years*365+leap-1;
	s += static_cast<u64>(days)*3600*24;
	return s;
}

tm TaskNet::sec2date(u64 s)
{
	const u32 CENTURY = 3155673600;
	int leap;
	int years0;
	if (s>=CENTURY)
	{
		leap = 1;
		years0 = 100;
		s -= CENTURY;
	}
	else
	{
		leap = 0;
		years0 = 0;
	}

	tm dt;
	dt.tm_sec = s%60;
	s /= 60;
	dt.tm_min = s%60;
	s /= 60;
	dt.tm_hour = s%24;
	s /= 24;

	int days = s%365;
	dt.tm_year = s/365;

	leap += dt.tm_year/4;
	leap -= dt.tm_year/100;
	leap += dt.tm_year/400;
	if(leap!=0 && (dt.tm_year%400==0 || (dt.tm_year%4==0 && dt.tm_year%100!=0)))
	{
		leap--;
	}

	while(days<leap)
	{
		dt.tm_year--;
		days+=365;
		if(dt.tm_year%400==0 || (dt.tm_year%4==0 && dt.tm_year%100!=0))
		{
			days++;
		}
	}
	days -= leap-1;

	dt.tm_mon = 0;
	while(days>DAYS_IN_MONTH[dt.tm_mon])
	{
		days -= DAYS_IN_MONTH[dt.tm_mon];
		if(dt.tm_mon==1 && (dt.tm_year+years0) && (dt.tm_year%400==0 || (dt.tm_year%4==0 && dt.tm_year%100!=0)))
		{
			if(days>1)
				days--;
			else
			{
				days = 29;
				dt.tm_mon = 1;
				break;
			}
		}
		dt.tm_mon++;
	}
	dt.tm_mday = days;
	dt.tm_year += years0;
	return dt;
}

void TaskNet::addTimezone(tm& dt, int timezone)
{
	dt.tm_hour += timezone;
	if(dt.tm_hour>24)
	{
		dt.tm_hour -= 24;
		dt.tm_mday++;
		if(dt.tm_mon!=1 && DAYS_IN_MONTH[dt.tm_mon]<dt.tm_mday)
		{
			dt.tm_mday = 1;
			dt.tm_mon++;
		}
		else if(dt.tm_mon==1)
		{
			if(dt.tm_mday==30 ||
				(dt.tm_mday==29 && dt.tm_year%400!=0 && (dt.tm_year%4!=0 || dt.tm_year%100==0)))
			{
				dt.tm_mday = 1;
				dt.tm_mon++;
			}
		}

		if(dt.tm_mon>=12)
		{
			dt.tm_mon = 0;
			dt.tm_year++;
		}
	}
	else if(dt.tm_hour<0)
	{
		dt.tm_hour += 24;
		dt.tm_mday--;
		if(dt.tm_mday<=0)
		{
			dt.tm_mon--;
			if(dt.tm_mon<0)
			{
				dt.tm_mon = 11;
				dt.tm_year--;
			}

			if(dt.tm_mon==1 && (dt.tm_year%400==0 || (dt.tm_year%4==0 && dt.tm_year%100!=0)))
			{
				dt.tm_mday=29;
			}
			else
			{
				dt.tm_mday = DAYS_IN_MONTH[dt.tm_mon];
			}
		}
	}
}

bool TaskNet::isTimeValid(const tm& dt)
{
	return dt.tm_year>=124 && dt.tm_year<200 &&
			dt.tm_mon>=0 && dt.tm_mon<12 &&
			dt.tm_mday>=1 && dt.tm_mday<=31 &&
			dt.tm_hour>=0 && dt.tm_hour<24 &&
			dt.tm_min>=0 && dt.tm_min<60 &&
			dt.tm_sec>=0 && dt.tm_sec<60;
}
