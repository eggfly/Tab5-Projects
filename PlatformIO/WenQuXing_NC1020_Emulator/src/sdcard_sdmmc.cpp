
#include <Arduino.h>
#include <SD_MMC.h>
#include "wqx_config.h"
#include "config.h"

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


void sdcard_sdmmc_init() {
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
    listDir(MY_SD, "/", 3);
}
