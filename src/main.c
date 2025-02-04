#include <assets.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <webp/demux.h>

#include "display.h"
#include "flash.h"
#include "gfx.h"
#include "remote.h"
#include "sdkconfig.h"
#include "wifi.h"

static const char* TAG = "main";
int32_t isAnimating = 5;  // Initialize with a valid value enough time for boot animation
int32_t app_dwell_secs = TIDBYT_REFRESH_INTERVAL_SECONDS;
char brightness_url[256];

// GPT HTTP SERVER CODE
// #include "esp_http_server.h"
// #include "esp_wifi.h"
// #include "nvs_flash.h"
// #include "driver/gpio.h"

// #define WIFI_SSID "YOUR_SSID"
// #define WIFI_PASS "YOUR_PASSWORD"
// #define LED_GPIO GPIO_NUM_2

// httpd_handle_t server = NULL;  // Global server handle

// esp_err_t trigger_handler(httpd_req_t* req) {
//   gpio_set_level(LED_GPIO, !gpio_get_level(LED_GPIO));  // Toggle LED
//   httpd_resp_send(req, "OK", 2);
//   return ESP_OK;
// }

// void start_server() {
//   httpd_config_t config = HTTPD_DEFAULT_CONFIG();
//   httpd_start(&server, &config);
//   httpd_uri_t trigger = {
//       .uri = "/trigger", .method = HTTP_GET, .handler = trigger_handler};
//   httpd_register_uri_handler(server, &trigger);
// }

// void wifi_init() {
//   esp_netif_init();
//   esp_event_loop_create_default();
//   esp_netif_create_default_wifi_sta();
//   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//   esp_wifi_init(&cfg);
//   wifi_config_t wifi_config = {
//       .sta = {.ssid = WIFI_SSID, .password = WIFI_PASS}};
//   esp_wifi_set_mode(WIFI_MODE_STA);
//   esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
//   esp_wifi_start();
// }

// void app_main() {
//   nvs_flash_init();
//   gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
//   wifi_init();
//   start_server();

//   // Main loop: Handle HTTP and other tasks
//   for (;;) {
//     httpd_conn_poll(server, 10);    // Process HTTP requests
//     vTaskDelay(pdMS_TO_TICKS(10));  // Prevent watchdog resets

//     // Your existing tasks
//     ESP_LOGW("Main", "Loop running...");
//   }
// }

void app_main(void) {
  ESP_LOGI(TAG, "Hello world!");

  // Setup the device flash storage.
  if (flash_initialize()) {
    ESP_LOGE(TAG, "failed to initialize flash");
    return;
  }
  esp_register_shutdown_handler(&flash_shutdown);

  // Setup the display.
  if (gfx_initialize(ASSET_NOAPPS_WEBP, ASSET_NOAPPS_WEBP_LEN)) {
    ESP_LOGE(TAG, "failed to initialize gfx");
    return;
  }
  esp_register_shutdown_handler(&display_shutdown);

  // Setup WiFi.
  if (wifi_initialize(TIDBYT_WIFI_SSID, TIDBYT_WIFI_PASSWORD)) {
    ESP_LOGE(TAG, "failed to initialize WiFi");
    return;
  }
  esp_register_shutdown_handler(&wifi_shutdown);

  char url[256] = TIDBYT_REMOTE_URL;

  // Build brightness URL by replacing "next" with "brightness"
  char* replace = strstr(url, "next");
  if (replace) {
    snprintf(brightness_url, sizeof(brightness_url), "%.*sbrightness%s",
             (int)(replace - url), url, replace + strlen("next"));
    ESP_LOGI("URL", "Updated: %s", brightness_url);
  } else {
    ESP_LOGW("URL", "Keyword 'next' not found in URL.");
  }

  ESP_LOGW(TAG,"Main Loop Start");
  for (;;) {
    // implement later
    // httpd_conn_poll(server, 10);  // Process HTTP requests
    uint8_t* webp;
    size_t len;
    static int brightness = DISPLAY_DEFAULT_BRIGHTNESS;

    if (remote_get(TIDBYT_REMOTE_URL, &webp, &len, &brightness, &app_dwell_secs)) {
      ESP_LOGE(TAG, "Failed to get webp");
      vTaskDelay(pdMS_TO_TICKS(1 * 1000));

    } else {
      // Successful remote_get
      ESP_LOGI(TAG, "Queuing webp (%d bytes)", len);
      gfx_update(webp, len);
      free(webp);
      if (brightness > -1 && brightness < 256) {
        display_set_brightness(brightness);
      }
      // Wait for app_dwell_secs to expire (isAnimating will be 0)
      if (isAnimating > 0 ) ESP_LOGW(TAG,"delay for animation");
      while ( isAnimating > 0 ) {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
      ESP_LOGW(TAG,"set isAnim=app_dwell_secs ; done delay for animation");
      isAnimating = app_dwell_secs; // use isAnimating as the container for app_dwell_secs
      
    }

  }

}