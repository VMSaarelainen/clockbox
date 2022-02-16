#include <stdint.h>
#include "string.h"
#include <string>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../components/Delay/src/ESP32/delay.h"
#include "../components/Gpio/src/ESP32/Gpio.h"

#include "../components/statuscodesandexceptions/src/StatusCodes.h"

#include "esp_http_server.h"
#include "ConfigAP.hpp"

//display includes
#include "../../components/i2c/src/ESP32/esp32_i2c.h"
#include "../../components/ssd1306display/src/SSD1306.h"

//pin defines
#define I2C_SDA_PIN Gpio_t::NUM_5
#define I2C_SCL_PIN Gpio_t::NUM_4


Delay delay = Delay();
Gpio *io = &Gpio::GetInstance();

ESP32I2c i2c = ESP32I2c();
SSD1306 display = SSD1306();

const int DEBUG_EPOCH_TIME = 1631011754;

int clockFaceRadius = 12;
int displayMiddleX = 64;
int displayMiddleY = 32;

//tick rate (framerate) in ms, also influences rng function call interval
int tickRate = 20;
//variables for rng function
double FactorTotal = 0.3;
double Factor1 = -3.2;
double Scale1 = -1.3;
double FactorE = -1.2;
double ScaleE = -1.7;
double FactorPi = 1.9;
double ScalePi = 0.7;

void webserver_run()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ConfigAP AP;
    //todo: set ssid and pw
    AP.Init();
}

void setTime(int curEpochTime)
{
    timeval tv = timeval();
    tv.tv_sec = curEpochTime;
    tv.tv_usec = 0;
    setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);      // Europe/Helsinki timezone
    tzset();                                              //set timezone
    settimeofday(const_cast<const timeval *>(&tv), NULL); //set system time
}

/**
 * @brief update clock time, returns true on succesfull read
 * 
 * @param OutHours out param for hours
 * @param OutMinutes out param for minutes
 * @param OutSeconds out param for seconds
 * @return true read succesfull
 * @return false read failed
 */
bool tick(int *OutHours, int *OutMinutes, int *OutSeconds)
{
    time_t now = time(NULL);
    tm *now_tm = localtime(&now);
    if (now_tm == nullptr)
    {
        ESP_LOGE("main", "local time read failed, errno: %i", errno);
        return false;
    }
    *OutHours = now_tm->tm_hour;
    *OutMinutes = now_tm->tm_min;
    *OutSeconds = now_tm->tm_sec;
    //ESP_LOGI("main", "Time: %i:%i:%i", *OutHours, *OutMinutes, *OutSeconds);
    /*if (gettimeofday(&tv, NULL) == 0) {       //epoch time print
        ESP_LOGI("main", "Epoch time: %ld", tv.tv_sec);
    }*/
    return true;
}

void calculateClockHands(int hours, int minutes, int seconds, double offsetAngle, double *hoursAngle, double *minutesAngle, double *secondsAngle)
{
    *secondsAngle = seconds * (2 * M_PI / 60) + offsetAngle;
    *minutesAngle = minutes * (2 * M_PI / 60) + offsetAngle;
    *hoursAngle = hours /12 * (2 * M_PI / 24) + (*minutesAngle / 24) + offsetAngle;
}

void drawClockFace(int hours, int minutes, int seconds, double offsetAngle)
{
    double hoursAngle = 0;
    double minutesAngle = 0;
    double secondsAngle = 0;
    calculateClockHands(hours, minutes, seconds, offsetAngle, &hoursAngle, &minutesAngle, &secondsAngle);

    display.clear();
    display.drawCircle(displayMiddleX, displayMiddleY, clockFaceRadius);
    //display.clearPixel(displayMiddleX + (clockFaceRadius * cos(offsetAngle)), displayMiddleY + (clockFaceRadius * sin(offsetAngle)));
    display.drawLine(   //dash indicating top of clock
        displayMiddleX + (clockFaceRadius * cos(offsetAngle)),
        displayMiddleY + (clockFaceRadius * sin(offsetAngle)),
        displayMiddleX + ((clockFaceRadius -2) * cos(offsetAngle)),
        displayMiddleY + ((clockFaceRadius -2) * sin(offsetAngle))
    );
    display.drawLine(displayMiddleX, displayMiddleY, displayMiddleX + clockFaceRadius /2 * cos(hoursAngle), displayMiddleY + clockFaceRadius /2 * sin(hoursAngle)); //hour hand
    display.drawLine(displayMiddleX, displayMiddleY, displayMiddleX + clockFaceRadius * cos(minutesAngle), displayMiddleY + clockFaceRadius * sin(minutesAngle));   //minute hand
    //display.drawLine(displayMiddleX, displayMiddleY, displayMiddleX + clockFaceRadius * cos(secondsAngle), displayMiddleY + clockFaceRadius * sin(secondsAngle));   //second hand
    display.display();
    //ESP_LOGI("ANGLES", "%f %f %f %f", hoursAngle, minutesAngle, secondsAngle, offsetAngle);
}
    
extern "C" void app_main(void)
{
    //webserver_run();
    StatusCode_t status = StatusCode_t::OK;
    //init
    status = i2c.Init(I2C_NUM_0, I2C_SDA_PIN, I2C_SCL_PIN, true);
    if (status != StatusCode_t::OK)
    {
        ESP_LOGI("main", "Failed i2c init %i", (int)status);
    }
    display.Init(0x3c, &i2c, &delay, nullptr);
    display.setFont(ArialMT_Plain_10);


    setTime(DEBUG_EPOCH_TIME);

    double offsetAngle = 0.0;
    double offsetAngleSeed = 1.0;   //todo: make rng its own function or class and init with random seed
    while (true)
    { //main loop
        int hours = 0;
        int minutes = 0;
        int seconds = 0;
        //offsetAngle = fmod((offsetAngle + M_PI/12), (2*M_PI));    //constant rotation

        //random wave generator function from https://stackoverflow.com/questions/8798771/perlin-noise-for-1d
        offsetAngle = FactorTotal * ( Factor1 * sin(Scale1 * offsetAngleSeed) - FactorE * sin(ScaleE * M_E * offsetAngleSeed) + FactorPi * sin(ScalePi * M_PI * offsetAngleSeed));
        offsetAngleSeed = offsetAngleSeed + 0.006;
        if (tick(&hours, &minutes, &seconds))
        {
            drawClockFace(hours, minutes, seconds, offsetAngle);
        }
        delay.ms(tickRate);
    }
}
