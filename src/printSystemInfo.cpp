  #include <Arduino.h>
  /**
   * Prints system info to the serial monitor as shown below:
   * 
   *  --- System Info ---
   *  model          ESP32
   *  cores          2
   *  features       WIFI b/g/n   BLE   BT
   *  revision       3
   *  full_revision  301
   *  chipId         0xaabf713c  
  */
  void printSystemInfo()
  {                         //   0       1        2           3       4       5           6           7       8       9
    const char *chipModel[] = {"none", "ESP32", "ESP32-S2", "none", "none", "ESP32-C3", "ESP32-H2", "none", "none", "ESP32-S3"};
    const char *chipFeatures[] = {"FLASH", "WIFI b/g/n", "", "", "BLE", "BT", "IEEE 802.15.4", "PSRAM"};
    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    uint32_t b = 1;

    Serial.println("\n--- System Info ---");
    Serial.printf("model          %s\ncores          %d\n", chipModel[chipInfo.model], chipInfo.cores);
    Serial.print("features       ");
    for (int i = 0; i < 8; i++)
    { // output the bit-coded features
      if (chipInfo.features & b) Serial.printf("%s   ", chipFeatures[i]);
      b <<= 1; // mask the next bit
    }
    Serial.printf("\nrevision       %d\nfull_revision  %d\nchipId         %#010x\n", 
                    chipInfo.revision, chipInfo.full_revision, ESP.getEfuseMac());
    Serial.printf("stackHighWaterMark %d\n", uxTaskGetStackHighWaterMark(NULL));
    Serial.println("-------------------");
  }