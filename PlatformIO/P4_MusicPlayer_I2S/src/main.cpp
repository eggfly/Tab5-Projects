#include "Arduino.h"
#include "SPI.h"
#include "FS.h"
#include "Wire.h"
#include "ES8388.h"
#include "Audio.h"

#include <vector>
#include <unordered_set>

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

#define USE_EXTERNAL_AUDIO_MODULE

// 还是外置Audio模块音质更好点
#ifdef USE_EXTERNAL_AUDIO_MODULE

#define I2S_DSIN 51 // pin not used
#define I2S_BCLK 48
#define I2S_LRC 2
#define I2S_MCLK 35
#define I2S_DOUT 47

#else

#define I2S_DSIN 28 // pin not used
#define I2S_BCLK 27
#define I2S_LRC 29
#define I2S_MCLK 30
#define I2S_DOUT 26
#endif

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

ES8388 dac; // ES8388 (new board)

#define es dac

int volume = 40; // 0...100

Audio audio;

bool shuffle_mode = true;

#define LIST_DIR_RECURSION_DEPTH 5
const bool APP_DEBUG = false;

std::vector<String> m_songFiles{};
int m_activeSongIdx{-1};

std::unordered_set<int> m_played_songs{};

// 0.5 代表随机去掉一半, 1.0 代表全都去掉, 越大去掉越多
float randomSkipRatio = 0.0;
size_t totalSongs = 0;

void stopSongWithMute()
{
    es.mute(ES8388::ES_MAIN, true);
    es.mute(ES8388::ES_OUT1, true);
    es.mute(ES8388::ES_OUT2, true);
    audio.stopSong();
}

void unmute()
{
    es.mute(ES8388::ES_MAIN, false);
    es.mute(ES8388::ES_OUT1, false);
    es.mute(ES8388::ES_OUT2, false);
}

void startNextSong(bool isNextOrPrev);

void autoPlayNextSong()
{
    if (m_songFiles.size() == 0)
    {
        delay(100);
        return;
    }
    if (!audio.isRunning())
    {
        Serial.println("autoPlay: playNextSong()");
        startNextSong(true);
    }
}

int strncmpci(const char *str1, const char *str2, size_t num)
{
    int ret_code = 0;
    size_t chars_compared = 0;

    if (!str1 || !str2)
    {
        ret_code = INT_MIN;
        return ret_code;
    }

    while ((chars_compared < num) && (*str1 || *str2))
    {
        ret_code = tolower((int)(*str1)) - tolower((int)(*str2));
        if (ret_code != 0)
        {
            break;
        }
        chars_compared++;
        str1++;
        str2++;
    }

    return ret_code;
}

bool startsWithIgnoreCase(const char *pre, const char *str)
{
    return strncmpci(pre, str, strlen(pre)) == 0;
}

bool endsWithIgnoreCase(const char *base, const char *str)
{
    int blen = strlen(base);
    int slen = strlen(str);
    return (blen >= slen) && (0 == strncmpci(base + blen - slen, str, strlen(str)));
}

void startNextSong(bool isNextOrPrev)
{
    if (m_songFiles.size() == 0)
    {
        return;
    }
    m_played_songs.insert(m_activeSongIdx);
    if (m_played_songs.size() * 2 > m_songFiles.size())
    {
        Serial.println("re-shuffle.");
        m_played_songs.clear();
    }
    if (isNextOrPrev)
    {
        m_activeSongIdx++;
    }
    else
    {
        m_activeSongIdx--;
    }
    if (shuffle_mode)
    {
        do
        {
            m_activeSongIdx = random(m_songFiles.size());
        } while (m_played_songs.find(m_activeSongIdx) != std::end(m_played_songs));
    }
    //  if (m_activeSongIdx >= m_songFiles.size() || m_activeSongIdx < 0) {
    //    m_activeSongIdx = 0;
    //  }
    m_activeSongIdx %= m_songFiles.size();
    Serial.print("songIndex=");
    Serial.print(m_activeSongIdx);
    Serial.print(", total=");
    Serial.println(m_songFiles.size());

    if (audio.isRunning())
    {
        stopSongWithMute();
        Serial.println("stop song");
        delay(2000);
        Serial.println("start next song");
    }
    // walkaround
    // setupButtonsNew();
    audio.connecttoFS(MY_SD, m_songFiles[m_activeSongIdx].c_str());

    Serial.println(m_songFiles[m_activeSongIdx].c_str());
}

void populateMusicFileList(String path, size_t depth)
{
    Serial.printf("search: %s, depth=%d\n", path.c_str(), LIST_DIR_RECURSION_DEPTH - depth);
    File musicDir = MY_SD.open(path);
    bool nextFileFound;
    do
    {
        nextFileFound = false;
        File entry = musicDir.openNextFile();
        if (entry)
        {
            nextFileFound = true;
            if (!entry.name() || entry.name()[0] == '.')
            {
                continue;
            }
            if (entry.isDirectory())
            {
                if (depth)
                {
                    populateMusicFileList(entry.path(), depth - 1);
                }
            }
            else
            {
                const bool entryIsFile = entry.size() > 4096;
                if (entryIsFile)
                {
                    if (APP_DEBUG)
                    {
                        Serial.print(entry.path());
                        Serial.print(" size=");
                        Serial.println(entry.size());
                    }
                    if (endsWithIgnoreCase(entry.name(), ".mp3") || endsWithIgnoreCase(entry.name(), ".flac") || endsWithIgnoreCase(entry.name(), ".aac") || endsWithIgnoreCase(entry.name(), ".wav"))
                    {
                        totalSongs++;
                        long r = random(10000);
                        bool keep = r > 10000 * randomSkipRatio;
                        if (keep)
                        {
                            m_songFiles.push_back(entry.path());
                        }
                    }
                }
            }
            entry.close();
        }
    } while (nextFileFound);
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
    if (!MY_SD.begin("/sdcard", use1BitMode, false, 80000))
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

    es.volume(ES8388::ES_MAIN, volume);
    es.volume(ES8388::ES_OUT1, 100);
    es.volume(ES8388::ES_OUT2, 100);
    // dac.SetVolumeHeadphone(volume);
    // dac.DumpRegisters();

    // Enable amplifier
    pinMode(GPIO_PA_EN, OUTPUT);
    digitalWrite(GPIO_PA_EN, HIGH);

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
    audio.setVolume(21); // 0...21

    auto populateStart = millis();

    Serial.printf("PSRAM剩余大小: %d字节\n", ESP.getFreePsram());
    populateMusicFileList("/", LIST_DIR_RECURSION_DEPTH);
    Serial.printf("PSRAM剩余大小: %d字节\n", ESP.getFreePsram());
    auto cost = millis() - populateStart;
    Serial.printf("populateMusicFileList cost %d ms, keep %d songs, total %d songs\n", cost, m_songFiles.size(), totalSongs);

    // 阳光照进回忆里
    // audio.connecttohost("http://42.193.120.65:8002/%E9%80%83%E8%B7%91%E8%AE%A1%E5%88%92-%E9%98%B3%E5%85%89%E7%85%A7%E8%BF%9B%E5%9B%9E%E5%BF%86%E9%87%8C.mp3");
    // audio.connecttoFS(SD_MMC, "/二十一世纪，当我们还年轻时.flac");
    // audio.connecttoFS(MY_SD, "/server/达达-Song F.mp3");
    // audio.connecttoFS(MY_SD, "/五月天/最重要的小事.mp3");
    // audio.connecttoFS(SD_MMC, "/渡口.wav");
    // audio.connecttoFS(SD_MMC, "/ygzjhyl.mp3");

    // audio.connecttohost("http://mp3channels.webradio.antenne.de:80/oldies-but-goldies");
    //  audio.connecttohost("http://dg-rbb-http-dus-dtag-cdn.cast.addradio.de/rbb/antennebrandenburg/live/mp3/128/stream.mp3");
    //  audio.connecttospeech("Wenn die Hunde schlafen, kann der Wolf gut Schafe stehlen.", "de");
}

void parseSerialCommand()
{
    if (Serial.available())
    {
        Serial.setTimeout(50);
        String r = Serial.readStringUntil('\n');
        r.trim();
        if (r.equalsIgnoreCase("n"))
        {
            Serial.println("play next song");
            startNextSong(true);
        }
        else if (r.equalsIgnoreCase("p"))
        {
            Serial.println("play previous song");
            startNextSong(false);
        }
        else if (r.equalsIgnoreCase("r"))
        {
            // toggle random shuffle mode
            shuffle_mode = !shuffle_mode;
            Serial.printf("shuffle mode: %s\n", shuffle_mode ? "on" : "off");
        }
        else if (r.equalsIgnoreCase("s"))
        {
            stopSongWithMute();
            Serial.println("stop song");
        }
        else if (r.equalsIgnoreCase("p"))
        {
            audio.pauseResume();
            Serial.println("pause/resume song");
        }
        else if (r.equalsIgnoreCase("+") || r.equalsIgnoreCase("="))
        {
            volume += 5;
            if (volume > 100)
            {
                volume = 100;
            }
            if (volume > 0)
            {
                unmute();
            }
            es.volume(ES8388::ES_MAIN, volume);
            Serial.printf("volume up: %d\n", volume);
        }
        else if (r.equalsIgnoreCase("-"))
        {
            volume -= 5;
            if (volume < 0)
            {
                volume = 0;
            }
            if (volume == 0)
            {
                es.mute(ES8388::ES_MAIN, true);
                es.mute(ES8388::ES_OUT1, true);
                es.mute(ES8388::ES_OUT2, true);
            }
            es.volume(ES8388::ES_MAIN, volume);
            Serial.printf("volume down: %d\n", volume);
        }
        else if (r.equalsIgnoreCase("info"))
        {
            Serial.println("Audio info:");
            Serial.printf("  codec: %s\n", audio.getCodecname());
            Serial.printf("  sample rate: %d\n", audio.getSampleRate());
            Serial.printf("  bits per sample: %d\n", audio.getBitsPerSample());
            Serial.printf("  channels: %d\n", audio.getChannels());
            Serial.printf("  bitrate: %d\n", audio.getBitRate());
            Serial.printf("  file size: %d\n", audio.getFileSize());
            Serial.printf("  file pos: %d\n", audio.getFilePos());
            Serial.printf("  file duration: %d sec\n", audio.getAudioFileDuration());
        }
        else if (r.equalsIgnoreCase("free"))
        {
            Serial.printf("free heap=%i, free psram=%i\n", ESP.getFreeHeap(), ESP.getFreePsram());
        }
        else if (r.equalsIgnoreCase("list"))
        {
            Serial.println("list songs:");
            for (int i = 0; i < m_songFiles.size(); i++)
            {
                Serial.printf("%d: %s\n", i, m_songFiles[i].c_str());
            }
        }
        else if (r.length() > 5)
        {
            // put streamURL in serial monitor
            stopSongWithMute();
            audio.connecttoFS(MY_SD, r.c_str());
        }
        // log_i("free heap=%i", ESP.getFreeHeap());
    }
}

void handleButton()
{
    if (digitalRead(0) == LOW)
    {
        while (digitalRead(0) == LOW)
        {
            delay(1);
        }
        Serial.println("play next song");
        startNextSong(true);
    }
}

void loop()
{
    delayMicroseconds(1);
    // handleButton();
    audio.loop();
    auto start = millis();
    parseSerialCommand();
    auto cost = millis() - start;
    if (cost > 500)
    {
        Serial.printf("%d ms\n", cost);
    }
    autoPlayNextSong();
}

// optional

void audio_info(const char *info)
{
    // if info starts with BitRate
    if (strncmp(info, "BitRate", 7) == 0)
    {
        unmute();
    }
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