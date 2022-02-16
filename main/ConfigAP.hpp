#pragma once
#include <string>
#include "esp_event.h"

class ConfigAP
{
    private:
        std::string ssid = "VM klocka";     //todo: read defaults from config (flash)
        std::string pw = "AAAA";
        static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data);
        void wifi_init_softap(const std::string& ssid, const std::string& pw);

    public:
        void Init();
        void Deinit();
        void Restart();
        void setSSID(const std::string& SSID);
        void setPW(const std::string& PW);
};