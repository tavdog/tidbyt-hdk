#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#define NVS_NAMESPACE "config_storage"  // Namespace for configurations
static const char* NVSTAG = "NVS";

void write_defaults_if_nvs_empty(void) {
  esp_err_t err;

  // Initialize NVS
  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(NVSTAG, "NVS flash needs to be erased");
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  // Open NVS handle
  nvs_handle_t nvs_handle;
  err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGE(NVSTAG, "Error opening NVS handle: %s", esp_err_to_name(err));
    return;
  }

  // Define the key-value pairs you want to store
  const char* wifi_ssid = TIDBYT_WIFI_SSID;
  const char* wifi_password = TIDBYT_WIFI_PASSWORD;
  const char* img_url = TIDBYT_REMOTE_URL;
  int8_t brightness = TIDBYT_DEFAULT_BRIGHTNESS;
  int8_t interval = TIDBYT_REFRESH_INTERVAL_SECONDS;
  // int8_t  = TIDBYT_DEFAULT_BRIGHTNESS
  // int32_t device_mode = 1;  // Example mode: 1 for auto, 0 for manual

  // Check and write each key-value pair
  size_t required_size = 0;

  // Check if `img_url` is stored
  err = nvs_get_str(nvs_handle, "img_url", NULL, &required_size);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(NVSTAG, "img_url not found. Writing default values.");
    err = nvs_set_str(nvs_handle, "wifi_ssid", wifi_ssid);
    nvs_set_str(nvs_handle, "wifi_password", wifi_password);
    nvs_set_str(nvs_handle, "img_url", img_url);
    nvs_set_i32(nvs_handle, "brightness", brightness);
    nvs_set_i32(nvs_handle, "interval", interval);
    if (err == ESP_OK) {
      ESP_LOGI(NVSTAG, "defaults written successfully.");
    } else {
      ESP_LOGE(NVSTAG, "Error writing defaults: %s", esp_err_to_name(err));
    }
  } else if (err == ESP_OK) {
    ESP_LOGI(NVSTAG, "img_url already exists. Not writing defaults.");
  }

  // Commit changes to NVS
  err = nvs_commit(nvs_handle);
  if (err == ESP_OK) {
    ESP_LOGI(NVSTAG, "NVS commit successful.");
  } else {
    ESP_LOGE(NVSTAG, "Error committing changes: %s", esp_err_to_name(err));
  }

  // Close NVS handle
  nvs_close(nvs_handle);
}