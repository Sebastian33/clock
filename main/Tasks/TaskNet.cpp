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

void TaskNet::NtpSync()
{
	hostent* info = gethostbyname("time.google.com");
	if(info->h_addr_list==nullptr)
	{
		return;
	}
	sockaddr_in addr;
	addr.sin_addr.s_addr = info->h_addr_list[0][0]|(info->h_addr_list[0][1]<<8)|(info->h_addr_list[0][2]<<16)|(info->h_addr_list[0][3]<<24);
	addr.sin_family = AF_INET;
	addr.sin_port = 123;
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
