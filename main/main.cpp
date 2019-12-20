



#include <stdio.h>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_event_loop.h"
#include "driver/sdmmc_host.h"
#include "driver/gpio.h"
#include "esp_vfs_semihost.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "esp_wifi.h"
#include "sdmmc_cmd.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mdns.h"
#include "sdkconfig.h"
#include "ESP32_SHT1x.h"
#include "sensor_manager.h"
#include "config_manager.h"
#include "infomanager.h"
#include "config_manager_defines.h"

#define CONFIG_EXAMPLE_WEB_MOUNT_POINT "/www"

// --- nice for developing purposes

#define xxUSE_DEFAULT_BOOTSTRAP 

////////////////////////////////////////////////////////////////////////////////////////

// --- these are our sensors (wrapped by a manager doing some calculation)

///SensorManager m_Sensors[CONFIG_TEMP_SENSOR_CNT];

////////////////////////////////////////////////////////////////////////////////////////

static const char *TAG = "ESPTempLogger";

////////////////////////////////////////////////////////////////////////////////////////

static void initialise_mdns(void)
{
    mdns_init();
    mdns_hostname_set(CONFIG_PRODUCT_NAME);
    mdns_instance_name_set("instance");

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}

////////////////////////////////////////////////////////////////////////////////////////

#define GOT_IPV4_BIT BIT(0)
#define GOT_IPV6_BIT BIT(1)

static EventGroupHandle_t s_connect_event_group;
static ip4_addr_t s_ip_addr;
static ip6_addr_t s_ipv6_addr;

////////////////////////////////////////////////////////////////////////////////////////

static void on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi go ip...");
    
    // --- now we are connected

    g_InfoManager.SetMode(InfoMode_Connected);
}

////////////////////////////////////////////////////////////////////////////////////////

static void on_got_ipv6(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi go ipv6...");

    //ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
    //memcpy(&s_ipv6_addr, &event->ip6_info.ip, sizeof(s_ipv6_addr));
    //xEventGroupSetBits(s_connect_event_group, GOT_IPV6_BIT);
}

////////////////////////////////////////////////////////////////////////////////////////

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    ESP_ERROR_CHECK(esp_wifi_connect());
}

////////////////////////////////////////////////////////////////////////////////////////

static void on_wifi_connect(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "on_wifi_connect...");
    tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
}

////////////////////////////////////////////////////////////////////////////////////////

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);

    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}


////////////////////////////////////////////////////////////////////////////////////////

// --- not: at least 8 chars!

#define DEFAULT_WIFI_PASS       "let-me-in-1234"

static void start_wifi_client()
{
    if (g_ConfigManager.GetIntValue(CFMGR_BOOTSTRAP_DONE) == 0)
    {   
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        uint8_t l_mac[6];

        // --- device NOT configured: create AP

        ESP_ERROR_CHECK(esp_read_mac(l_mac, ESP_MAC_WIFI_SOFTAP));

        char l_macstr[64];
        char l_prodname[32];

        strncpy(l_prodname,CONFIG_PRODUCT_NAME,32);

        snprintf(l_macstr,64,"%s-%02x%02x%02x%02x%02x%02x",l_prodname,MAC2STR(l_mac));

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

        wifi_config_t wifi_config;
        
        strncpy((char *)wifi_config.ap.ssid,l_macstr,32);
        strncpy((char *)wifi_config.ap.password,DEFAULT_WIFI_PASS,64);
        
        wifi_config.ap.ssid_len = (uint8_t)strlen(l_macstr);
        wifi_config.ap.max_connection = 5;
        wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

        if (strlen(DEFAULT_WIFI_PASS) == 0) 
        {
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        //ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s",(char *)wifi_config.ap.ssid);
    }
    else
    {
        // --- device configured: connecting to WLAN

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &on_wifi_connect, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &on_got_ipv6, NULL));

        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

        wifi_config_t wifi_config;
        memset(&wifi_config,0,sizeof(wifi_config_t));

        // --- get config from config manager

        std::string l_ssid      = g_ConfigManager.GetStringValue(CFMGR_WIFI_SSID);
        std::string l_wlanpwd   = g_ConfigManager.GetStringValue(CFMGR_WIFI_PASSWORD);

        strncpy((char *)wifi_config.sta.ssid,l_ssid.c_str(),32);
        strncpy((char *)wifi_config.sta.password,l_wlanpwd.c_str(),64);

        ESP_LOGI(TAG, "Connecting to '%s'...", wifi_config.sta.ssid);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
}

////////////////////////////////////////////////////////////////////////////////////////

static void stop_wifi_client()
{
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip));

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &on_got_ipv6));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &on_wifi_connect));

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
}

////////////////////////////////////////////////////////////////////////////////////////

esp_err_t init_fs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_EXAMPLE_WEB_MOUNT_POINT,
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}


////////////////////////////////////////////////////////////////////////////////////////

void init_sensors(void)
{
    g_SensorManager.InitSensors();
}

////////////////////////////////////////////////////////////////////////////////////////

void ProcessMeasurements(void)
{
    g_SensorManager.ProcessMeasurements();
}

////////////////////////////////////////////////////////////////////////////////////////

esp_err_t start_rest_server(const char *base_path);

// --- need this now since it is a cpp code file

extern "C" 
{
    void app_main();
}

// --- the main starting point

void app_main()
{
    // ---- init flash lib

    ESP_ERROR_CHECK(nvs_flash_init());

    // --- start the info manager

    g_InfoManager.InitInitManager();

    // ---- init config storage

    ESP_ERROR_CHECK(g_ConfigManager.InitConfigManager());

    // ---- init tcpip lib

    tcpip_adapter_init();
 
    // ---- create default event loop

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // ---- check for bootstrap mode

    if (g_ConfigManager.GetIntValue(CFMGR_BOOTSTRAP_DONE) == 0)
    {
        ESP_LOGI(TAG, "Device not bootstrapped - enter bootstrap mode");

        g_InfoManager.SetMode(InfoMode_Bootstrap);

#ifdef USE_DEFAULT_BOOTSTRAP
        
        ESP_LOGE(TAG, "Development - use Tim Std Config");

        g_ConfigManager.SetIntValue(CFMGR_BOOTSTRAP_DONE    ,1);
        g_ConfigManager.SetStringValue(CFMGR_WIFI_SSID      ,"Timbo_WLAN17_2G");
        g_ConfigManager.SetStringValue(CFMGR_WIFI_PASSWORD  ,"Adjust1234Hugendubel2008");
        g_ConfigManager.SetStringValue(CFMGR_DEVICE_NAME    ,"TimDevDevice");

#endif

    }
    else
    {
        g_InfoManager.SetMode(InfoMode_WaitToConnect);

        ESP_LOGI(TAG, "Device configuration found. Let's rock it.");
    }

    // ---- setup bonjour

    initialise_mdns();

    start_wifi_client();
    
    // ---- setup SPIFFS flashed in rom

    ESP_ERROR_CHECK(init_fs());
    
    // ---- initialize all the sensors

    init_sensors();

    // ---- now start the web server

    start_rest_server(CONFIG_EXAMPLE_WEB_MOUNT_POINT);

	// ---- main measurement loop

    while(1) 
    {
        // --- check for the user pressing the bootstrap key

        if (g_InfoManager.IsBootstrapActivated())  
        {
            ESP_LOGI(TAG, "Bootstrap activated by user. Reset bootstrap flag and reboot!");

            if (g_ConfigManager.GetIntValue(CFMGR_BOOTSTRAP_DONE) == 0)
            {
                ESP_LOGI(TAG, "Device not bootstrapped - ignore!");
            }
            else
            {
                ESP_LOGI(TAG, "Reset bootstrap flag and reboot!");

                // --- set the flash flag to "not configured"

                g_ConfigManager.SetIntValue(CFMGR_BOOTSTRAP_DONE,0);

                // --- be sure to let the flash write the stuff

                nvs_flash_deinit();

                // --- and reboot

                esp_restart();
            }
            
        }    

		ProcessMeasurements();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
