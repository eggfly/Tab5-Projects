#include "Arduino.h"
#include "SPI.h"
#include "FS.h"
#include "Wire.h"
#include "ES8388.h"
#include "Audio.h"

// #define ENABLE_SDCARD_SPI 1
// #define ENABLE_WIFI 1
#define ENABLE_SDCARD_SDMMC 1

#define MY_SD SD_MMC

#ifdef ENABLE_SDCARD_SPI
#include "SD.h"
#endif

#ifdef ENABLE_SDCARD_SDMMC
#include "SD_MMC.h"
#endif

#ifdef ENABLE_WIFI
#include "WiFi.h"
#endif

#ifdef ENABLE_SDCARD_SPI
// SPI GPIOs
#define SD_CS 13
#define SPI_MOSI 15
#define SPI_MISO 2
#define SPI_SCK 14
#endif

#ifdef ENABLE_SDCARD_SDMMC
#define SD_MMC_CLK 43
#define SD_MMC_CMD 44
#define SD_MMC_D0 39
#define SD_MMC_D1 40
#define SD_MMC_D2 41
#define SD_MMC_D3 42
#endif

// I2S GPIOs, the names refer on ES8388, AS1 Audio Kit V2.2 3378
#define I2S_DSIN 28 // pin not used
#define I2S_BCLK 27
#define I2S_LRC 29
#define I2S_MCLK 30
#define I2S_DOUT 26

// I2C GPIOs
#define IIC_CLK 32
#define IIC_DATA 31

// buttons
// #define BUTTON_2_PIN 13             // shared mit SPI_CS
// #define BUTTON_3_PIN 19
// #define BUTTON_4_PIN 23
// #define BUTTON_5_PIN 18 // Stop
// #define BUTTON_6_PIN 5  // Play

// amplifier enable
#define GPIO_PA_EN 21

// Switch S1: 1-OFF, 2-ON, 3-ON, 4-OFF, 5-OFF

String ssid = "WIFI***";
String password = "********";

ES8388 dac;      // ES8388 (new board)
int volume = 40; // 0...100

Audio audio;

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                if (file.name()[0] != '.')
                {
                    listDir(fs, file.path(), levels - 1);
                }
            }
        }
        else
        {
            // filter out file name which starts with "."
            if (file.name()[0] != '.')
            {
                Serial.print("  FILE: ");
                Serial.print(file.name());
                Serial.print("  SIZE: ");
                Serial.println(file.size());
            }
        }
        file = root.openNextFile();
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("\r\nReset");
    Serial.printf_P(PSTR("Free mem=%ld\n"), ESP.getFreeHeap());
#ifdef ENABLE_SDCARD_SDMMC
    const bool use1BitMode = false;
    if (use1BitMode)
    {
        MY_SD.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
    }
    else
    {
        MY_SD.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0, SD_MMC_D1, SD_MMC_D2, SD_MMC_D3);
    }
    if (!MY_SD.begin("/sdcard", use1BitMode, false, 40000))
    {
        Serial.println("Card Mount Failed");
        return;
    }
#endif

#ifdef ENABLE_SDCARD_SPI
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    SPI.setFrequency(1000000);

    SD.begin(SD_CS);
#endif

#ifdef ENABLE_WIFI
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    Serial.printf_P(PSTR("Connected\r\nRSSI: "));
    Serial.print(WiFi.RSSI());
    Serial.print(" IP: ");
    Serial.println(WiFi.localIP());
#endif

    Serial.printf("Connect to DAC codec... ");
    while (not dac.begin(IIC_DATA, IIC_CLK))
    {
        Serial.printf("Failed!\n");
        delay(1000);
    }
    Serial.printf("OK! ES8388 Audio Initialized!\n");

    dac.SetVolumeSpeaker(volume);
    dac.SetVolumeHeadphone(volume);
    // dac.DumpRegisters();

    // Enable amplifier
    pinMode(GPIO_PA_EN, OUTPUT);
    digitalWrite(GPIO_PA_EN, HIGH);

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
    audio.setVolume(15); // 0...21

    listDir(MY_SD, "/", 3);

    // 阳光照进回忆里
    // audio.connecttohost("http://42.193.120.65:8002/%E9%80%83%E8%B7%91%E8%AE%A1%E5%88%92-%E9%98%B3%E5%85%89%E7%85%A7%E8%BF%9B%E5%9B%9E%E5%BF%86%E9%87%8C.mp3");
    // audio.connecttoFS(SD_MMC, "/二十一世纪，当我们还年轻时.flac");
    // audio.connecttoFS(MY_SD, "/server/达达-Song F.mp3");
    audio.connecttoFS(MY_SD, "/五月天/最重要的小事.mp3");
    // audio.connecttoFS(SD_MMC, "/渡口.wav");
    // audio.connecttoFS(SD_MMC, "/ygzjhyl.mp3");

    // audio.connecttohost("http://mp3channels.webradio.antenne.de:80/oldies-but-goldies");
    //  audio.connecttohost("http://dg-rbb-http-dus-dtag-cdn.cast.addradio.de/rbb/antennebrandenburg/live/mp3/128/stream.mp3");
    //  audio.connecttospeech("Wenn die Hunde schlafen, kann der Wolf gut Schafe stehlen.", "de");
}

void loop()
{
    vTaskDelay(1);
    audio.loop();
}

// optional
void audio_info(const char *info)
{
    Serial.print("info        ");
    Serial.println(info);
}
void audio_id3data(const char *info)
{ // id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
}
void audio_eof_mp3(const char *info)
{ // end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
}
void audio_showstation(const char *info)
{
    Serial.print("station     ");
    Serial.println(info);
}
void audio_showstreamtitle(const char *info)
{
    Serial.print("streamtitle ");
    Serial.println(info);
}
void audio_bitrate(const char *info)
{
    Serial.print("bitrate     ");
    Serial.println(info);
}
void audio_commercial(const char *info)
{ // duration in sec
    Serial.print("commercial  ");
    Serial.println(info);
}
void audio_icyurl(const char *info)
{ // homepage
    Serial.print("icyurl      ");
    Serial.println(info);
}
void audio_lasthost(const char *info)
{ // stream URL played
    Serial.print("lasthost    ");
    Serial.println(info);
}
void audio_eof_speech(const char *info)
{
    Serial.print("eof_speech  ");
    Serial.println(info);
}