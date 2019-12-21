/* HTTP Restful API Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <fcntl.h>
#include <string>

#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_wifi.h"

#include "sensor_manager.h"
#include "config_manager.h"
#include "config_manager_defines.h"

////////////////////////////////////////////////////////////////////////////////////////

static const char *REST_TAG = "esp-rest";

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

#define DEFAULT_SCAN_LIST_SIZE 128

////////////////////////////////////////////////////////////////////////////////////////

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

////////////////////////////////////////////////////////////////////////////////////////

std::string SanetizedString(const char *f_s)
{
    std::string html(f_s);
    std::string text;

    for(;;)
    {
        std::string::size_type  startpos;

        startpos = html.find('<');
        if(startpos == std::string::npos)
        {
            // no tags left only text!
            text += html;
            break;
        }

        // handle the text before the tag    
        if(0 != startpos)
        {
            text += html.substr(0, startpos);
            html = html.substr(startpos, html.size() - startpos);
            startpos = 0;
        }

        //  skip all the text in the html tag

        std::string::size_type endpos;
        for(endpos = startpos; endpos < html.size() && html[endpos] != '>'; ++endpos)
        {
            // since '>' can appear inside of an attribute string we need
            // to make sure we process it properly.
            if(html[endpos] == '"')
            {
                endpos++;
                while(endpos < html.size() && html[endpos] != '"')
                {
                    endpos++;
                }
            }
        }

        //  Handle text and end of html that has beginning of tag but not the end
        if(endpos == html.size())
        {
            html = html.substr(endpos, html.size() - endpos);
            break;
        }
        else
        {
            //  handle the entire tag
            endpos++;
            html = html.substr(endpos, html.size() - endpos);
        }
    }

    return text;
}

////////////////////////////////////////////////////////////////////////////////////////

inline bool CheckFileExtension(const char *filename, const char *ext)
{
    return strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0;
}

////////////////////////////////////////////////////////////////////////////////////////

// ---- Set HTTP response content type according to file extension 

static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CheckFileExtension(filepath, ".html")) {
        type = "text/html";
    } else if (CheckFileExtension(filepath, ".js")) {
        type = "application/javascript";
    } else if (CheckFileExtension(filepath, ".css")) {
        type = "text/css";
    } else if (CheckFileExtension(filepath, ".png")) {
        type = "image/png";
    } else if (CheckFileExtension(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CheckFileExtension(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

////////////////////////////////////////////////////////////////////////////////////////

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    ESP_LOGI(REST_TAG,"rest_common_get_handler %s",req->uri);

    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/') {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////

#ifdef NOTUSED

/* Simple handler for getting system handler */
static esp_err_t system_info_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(root, "version", IDF_VER);
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////

static esp_err_t temperature_data_get_handler(httpd_req_t *req)
{
    ESP_LOGI(REST_TAG,"temperature_data_get_handler %s",req->uri);

    httpd_resp_set_type(req, "application/json");

    // ---- find trailing backslash

    char *l_sensorint = strrchr(req->uri,'/');
    if (!l_sensorint)
    {
        ESP_LOGE(REST_TAG, "temperature_data_get_handler: Illegal URI");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Illegal URI");
        return ESP_FAIL;
    }

    // ---- convert to int

    int l_sensor_idx = atoi(l_sensorint+1);

    // ---- check if this is a valid index

    if (l_sensor_idx < 1 && l_sensor_idx>= CONFIG_TEMP_SENSOR_CNT)
    {
        ESP_LOGE(REST_TAG, "temperature_data_get_handler: Illegal sensor index %d",l_sensor_idx);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Illegal sensor index");
        return ESP_FAIL;
    }

    // ---- now ask the sensor for the values and create a JSON from that    

    cJSON *root = cJSON_CreateObject();
    
    cJSON_AddNumberToObject(root, "temp", g_SensorManager.GetSensor(l_sensor_idx-1).GetTemp());
    cJSON_AddNumberToObject(root, "rh", g_SensorManager.GetSensor(l_sensor_idx-1).GetRH());
    cJSON_AddNumberToObject(root, "dp", g_SensorManager.GetSensor(l_sensor_idx-1).GetDP());
    
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    
    free((void *)sys_info);
    cJSON_Delete(root);
    
    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////

static esp_err_t temperature_cnt_get_handler(httpd_req_t *req)
{
    ESP_LOGI(REST_TAG,"temperature_cnt_get_handler %s",req->uri);

    httpd_resp_set_type(req, "application/json");

    // ---- just return the sensor count
    
    cJSON *root = cJSON_CreateObject();
    
    cJSON_AddNumberToObject(root, "cnt", CONFIG_TEMP_SENSOR_CNT);
    
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    
    free((void *)sys_info);
    cJSON_Delete(root);
    
    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////

static esp_err_t config_apscan_handler(httpd_req_t *req)
{
    ESP_LOGI(REST_TAG,"config_apscan_handler %s",req->uri);
    
    httpd_resp_set_type(req, "application/json");

    // ---- just return the sensor count
    
    cJSON *root = cJSON_CreateObject();
    
    cJSON *wifiscan_array = cJSON_AddArrayToObject(root,"WiFI_Scan");
    
    // --- initiate a wifi scan 

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t *ap_info = new wifi_ap_record_t[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset((void *)ap_info, 0, sizeof(wifi_ap_record_t) * DEFAULT_SCAN_LIST_SIZE);

    wifi_scan_config_t l_wscf;

    l_wscf.ssid  = NULL;             
    l_wscf.bssid = NULL;             
    l_wscf.channel = 0;             
    l_wscf.show_hidden = false;           
    l_wscf.scan_type = WIFI_SCAN_TYPE_ACTIVE;  
    l_wscf.scan_time.active.min = 200;  
    l_wscf.scan_time.active.min = 1500;  

    esp_err_t l_err;

    l_err = esp_wifi_scan_start(&l_wscf, true);
    if ( l_err != ESP_OK )
    {
        ESP_LOGE(REST_TAG,"Error on esp_wifi_scan_start: %d",l_err);
        return l_err;
    }

    l_err = esp_wifi_scan_get_ap_records(&number, ap_info);
    if ( l_err != ESP_OK )
    {
        ESP_LOGE(REST_TAG,"Error on esp_wifi_scan_get_ap_records: %d",l_err);
        return l_err;
    }

    l_err = esp_wifi_scan_get_ap_num(&ap_count);
    if ( l_err != ESP_OK )
    {
        ESP_LOGE(REST_TAG,"Error on esp_wifi_scan_get_ap_num: %d",l_err);
        return l_err;
    }

    ESP_LOGI(REST_TAG, "Total APs scanned = %u", ap_count);
 
    for (int i = 0; i < ap_count; i++) 
    {
        cJSON *l_s = cJSON_CreateString((char *)ap_info[i].ssid);
        cJSON_AddItemToArray(wifiscan_array,l_s);

        ESP_LOGI(REST_TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(REST_TAG, "RSSI \t\t%d", ap_info[i].rssi);
    }

    delete ap_info;

    // --- now create JSON and send back
    
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    
    free((void *)sys_info);
    cJSON_Delete(root);
    
    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////

static esp_err_t config_get_handler(httpd_req_t *req)
{
    ESP_LOGI(REST_TAG,"config_get_handler %s",req->uri);

    httpd_resp_set_type(req, "application/json");

    // ---- just return the sensor count
    
    cJSON *root = cJSON_CreateObject();
    
    cJSON_AddNumberToObject(root, CFMGR_BOOTSTRAP_DONE, g_ConfigManager.GetIntValue(CFMGR_BOOTSTRAP_DONE));
    cJSON_AddStringToObject(root, CFMGR_WIFI_SSID,      g_ConfigManager.GetStringValue(CFMGR_WIFI_SSID).c_str());
    cJSON_AddStringToObject(root, CFMGR_WIFI_PASSWORD,  g_ConfigManager.GetStringValue(CFMGR_WIFI_PASSWORD).c_str());
    cJSON_AddStringToObject(root, CFMGR_DEVICE_NAME,    g_ConfigManager.GetStringValue(CFMGR_DEVICE_NAME).c_str());

    cJSON_AddStringToObject(root, CFMGR_MQTT_SERVER,    g_ConfigManager.GetStringValue(CFMGR_MQTT_SERVER).c_str());
    cJSON_AddNumberToObject(root, CFMGR_MQTT_PORT,      g_ConfigManager.GetIntValue(CFMGR_MQTT_PORT));
    cJSON_AddStringToObject(root, CFMGR_MQTT_TOPIC,     g_ConfigManager.GetStringValue(CFMGR_MQTT_TOPIC).c_str());
    cJSON_AddNumberToObject(root, CFMGR_MQTT_TIME,      g_ConfigManager.GetIntValue(CFMGR_MQTT_TIME));
    cJSON_AddNumberToObject(root, CFMGR_MQTT_ENABLE,    g_ConfigManager.GetIntValue(CFMGR_MQTT_ENABLE));

    // --- now create JSON and send back
    
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    
    free((void *)sys_info);
    cJSON_Delete(root);
    
    return ESP_OK;
}


///////////////////////////////////////////////////////////////////////////////////////

esp_err_t ProcessJsonString(cJSON *f_root,const char *f_name, bool f_onlysetifnotempty = false)
{
    cJSON *l_js = cJSON_GetObjectItem(f_root, f_name);

    if (!l_js)
    {
        ESP_LOGE(REST_TAG, "Config %s not found", f_name);
        return ESP_FAIL;
    }

    const char *l_s = l_js->valuestring;
    if (!l_s)
    {
        ESP_LOGE(REST_TAG, "Config %s is null", f_name);
        return ESP_FAIL;
        
    }

    // --- if flag is set and string is empty - do noting

    if (f_onlysetifnotempty && strlen(l_s) == 0)
    {
        ESP_LOGI(REST_TAG, "Config %s empty - not set!", f_name);
        return ESP_OK;
    }

    // --- now tell the config mgr

    g_ConfigManager.SetStringValue(f_name,SanetizedString(l_s));
    ESP_LOGI(REST_TAG, "Config %s, value '%s'", f_name,l_s);

    return ESP_OK;
}

///////////////////////////////////////////////////////////////////////////////////////

esp_err_t ProcessJsonInt(cJSON *f_root,const char *f_name)
{
    cJSON *l_js = cJSON_GetObjectItem(f_root, f_name);

    if (!l_js)
    {
        ESP_LOGE(REST_TAG, "Config %s not found", f_name);
        return ESP_FAIL;
    }

    // --- now tell the config mgr

    g_ConfigManager.SetIntValue(f_name,l_js->valueint);
    ESP_LOGI(REST_TAG, "Config %s, value '%d'", f_name,l_js->valueint);

    return ESP_OK;
}

///////////////////////////////////////////////////////////////////////////////////////

static esp_err_t config_post_handler(httpd_req_t *req)
{
    ESP_LOGI(REST_TAG,"config_post_handler %s",req->uri);

    // --- check if we have enough space to process full post request

    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) 
    {
        // --- Respond with 500 Internal Server Error
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }

    // --- okay, now read the full request

    while (cur_len < total_len) 
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) 
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    // --- convert the JSON string to a JSON object

    cJSON *root = cJSON_Parse(buf);
    
    // --- set config to config mgr

    ProcessJsonString(root,CFMGR_WIFI_SSID);
    ProcessJsonString(root,CFMGR_WIFI_PASSWORD,true);
    ProcessJsonString(root,CFMGR_DEVICE_NAME);  
    ProcessJsonString(root,CFMGR_MQTT_SERVER);
    ProcessJsonString(root,CFMGR_MQTT_TOPIC);

    ProcessJsonInt(root,CFMGR_MQTT_PORT);
    ProcessJsonInt(root,CFMGR_MQTT_TIME);
    ProcessJsonInt(root,CFMGR_MQTT_ENABLE);

    // --- flag now
    
    g_ConfigManager.SetIntValue(CFMGR_BOOTSTRAP_DONE,1);

    // --- free up the JSON object

    cJSON_Delete(root);
    
    // --- send status to server

    httpd_resp_sendstr(req, "Post control value successfully");
    
    return ESP_OK;
}
////////////////////////////////////////////////////////////////////////////////////////

esp_err_t start_rest_server(const char *base_path)
{
    assert(base_path);

    rest_server_context_t *rest_context = (rest_server_context_t *)calloc(1, sizeof(rest_server_context_t));
    if (!rest_context)
    {
        ESP_LOGE(REST_TAG, "No memory for rest context");
        return ESP_FAIL;
    }
    
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_resp_headers = 16;

    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");

    if (httpd_start(&server, &config) != ESP_OK)
    {
        ESP_LOGE(REST_TAG, "Start server failed");
    }

    // ---- URI handler for getting the ap scan

    httpd_uri_t config_apscan_uri;
    
    config_apscan_uri.uri      = "/api/v1/apscan";
    config_apscan_uri.user_ctx = rest_context;
    config_apscan_uri.method   = HTTP_GET;
    config_apscan_uri.handler  = config_apscan_handler;
    
    httpd_register_uri_handler(server, &config_apscan_uri);

    // ---- URI handler for getting the current configuration

    httpd_uri_t config_get_uri;
    
    config_get_uri.uri      = "/api/v1/config";
    config_get_uri.user_ctx = rest_context;
    config_get_uri.method   = HTTP_GET;
    config_get_uri.handler  = config_get_handler;
    
    httpd_register_uri_handler(server, &config_get_uri);

    // ---- URI handler for setting the current configuration

    httpd_uri_t config_post_uri;
    
    config_post_uri.uri      = "/api/v1/config";
    config_post_uri.user_ctx = rest_context;
    config_post_uri.method   = HTTP_POST;
    config_post_uri.handler  = config_post_handler;
    
    httpd_register_uri_handler(server, &config_post_uri);

    // ---- URI handler for getting the number iof sensors

    httpd_uri_t temperature_cnt_get_uri;
    
    temperature_cnt_get_uri.uri      = "/api/v1/sensorcnt";
    temperature_cnt_get_uri.user_ctx = rest_context;
    temperature_cnt_get_uri.method   = HTTP_GET;
    temperature_cnt_get_uri.handler  = temperature_cnt_get_handler;
    
    httpd_register_uri_handler(server, &temperature_cnt_get_uri);

    // ---- URI handler for getting temperature

    httpd_uri_t temperature_data_get_uri;
    
    temperature_data_get_uri.uri      = "/api/v1/temp/*";
    temperature_data_get_uri.user_ctx = rest_context;
    temperature_data_get_uri.method   = HTTP_GET;
    temperature_data_get_uri.handler  = temperature_data_get_handler;
    
    httpd_register_uri_handler(server, &temperature_data_get_uri);

    // ---- URI handler for getting web server files 

    httpd_uri_t common_get_uri;
    
    common_get_uri.uri      = "/*";
    common_get_uri.user_ctx = rest_context;
    common_get_uri.method   = HTTP_GET;
    common_get_uri.handler  = rest_common_get_handler;
    
    httpd_register_uri_handler(server, &common_get_uri);

    return ESP_OK;
}
