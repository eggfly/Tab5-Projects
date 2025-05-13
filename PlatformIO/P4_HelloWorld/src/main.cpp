#include <Arduino.h>

#include <SD_MMC.h>

void setup()
{
       Serial.begin(115200);
       Serial.println("test");
}

void loop()
{
       // put your main code here, to run repeatedly:
       Serial.println("loop");
       delay(1000);
}
