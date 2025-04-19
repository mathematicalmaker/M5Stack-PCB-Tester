/*
 * PCB Testing System
 * Copyright (c) 2025 Jason Hoomani
 * Licensed under the MIT License. See LICENSE file for details.
 */

 #include <Arduino.h>
 #include <M5Unified.h>
 #include <Adafruit_MCP23X17.h>
 #include <map>
 #include <string>
 
 #define MCP_ADDRESS 0x27
 
 // -------- Constants --------
 constexpr const int MAX_NET_SIZE = 7; // Maximum number of pins in a net
 constexpr const int MAX_LAYOUTS = 3; // Number of different PCB layouts
 constexpr const int MAX_NETS = 8; // Maximum number of nets per layout
 
 Adafruit_MCP23X17 mcp;
 
 // -------- Pin Mapping (ONE mapping for all layouts) --------
 std::map<std::string, int> pinMapping = {
     {"2", 11},
     {"4", 12},
     {"6", 13},
     {"10", 14},
     {"12", 15},
     {"1", 4},
     {"3", 5},
     {"5", 6},
     {"7", 7},
     {"9", 8},
     {"11", 9},
     {"CN1", 0},
     {"CN2", 1},
     {"CN3", 2},
     {"CN4", 3}
 };
 
 const char* layoutNames[MAX_LAYOUTS] = {
   "DEM w/ Power Header",
   "Plain DEM",
   "DEM w/ Pwr Hdr Tray State"
 };
 
 // -------- Net Configuration for each Layout --------
 // Layout 1
 const char* netConfig1[][MAX_NET_SIZE + 1] = {
     {"4-7", "4", "7"},
     {"1-9-CN1|12V", "1", "9", "CN1"},
     {"2-10-CN4|5V", "2", "10", "CN4"},
     {"3-5-6-11-12|CN2-CN3 GND", "3", "5", "6", "11", "12", "CN2", "CN3"}
 };
 
 // Layout 2
 const char* netConfig2[][MAX_NET_SIZE + 1] = {
     {"4-7", "4", "7"},
     {"5-11", "5", "11"},
     {"6-12", "6", "12"}
 };
 
 // Layout 3 (example)
 const char* netConfig3[][MAX_NET_SIZE + 1] = {
     {"4-7", "4", "7"},
     {"5-6-11-12", "5", "6", "11", "12"}
 };
 
 // --- Array of pointers to netConfigs and their net counts ---
 const char* (*netConfigPtrs[MAX_LAYOUTS])[MAX_NET_SIZE + 1] = {
     netConfig1,
     netConfig2,
     netConfig3
 };
 const int netCounts[MAX_LAYOUTS] = {
     sizeof(netConfig1) / sizeof(netConfig1[0]),
     sizeof(netConfig2) / sizeof(netConfig2[0]),
     sizeof(netConfig3) / sizeof(netConfig3[0])
 };
 
 // --------- Globals for Current Layout ---------
 int currentLayoutIndex = 0;
 int NUM_NETS = netCounts[0];
 int netPins[MAX_NETS][MAX_NET_SIZE];
 
 const int circleRadius = 20;
 const int circleSpacingX = 120;
 const int circleSpacingY = 100;
 const int startX = 100;
 const int startY = 50;
 const int labelSpacing = 5;
 const int labelFontSize = 2;
 const int circlesPerRow = 2;
 
 int circleX[MAX_NETS];
 int circleY[MAX_NETS];
 
 // --------- Function Prototypes ---------
 void setupNets();
 bool testNet(const int *pins);
 bool testOtherNets(int currentNet);
 void updateCircle(int circleIndex, bool pass);
 void drawLabel(int x, int y, const char* label);
 void drawLabels();
 void selectLayout(int layoutIndex);
 void checkButtons();
 
 void setup() {
     Serial.begin(115200);
     M5.begin();
     M5.Display.fillScreen(TFT_BLACK);
 
     if (mcp.begin_I2C(MCP_ADDRESS)) {
         Serial.println("MCP23017 found!");
     } else {
         Serial.println("MCP23017 not found.");
         M5.Display.fillScreen(TFT_RED);
         M5.Display.setTextColor(TFT_WHITE, TFT_RED);
         M5.Display.setTextDatum(MC_DATUM);
         M5.Display.drawString("MCP23017 NOT FOUND", M5.Display.width() / 2, M5.Display.height() / 2);
         while (1);
     }
 
     // Configure display settings
     M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
     M5.Display.setTextFont(labelFontSize);
     M5.Display.setTextDatum(TC_DATUM);
 
     setupNets();
     drawLabels();
 }
 
 void loop() {
     checkButtons();
 
     bool netPass[MAX_NETS];
 
     for (int net = 0; net < NUM_NETS; net++) {
         mcp.pinMode(netPins[net][0], OUTPUT);
         mcp.digitalWrite(netPins[net][0], LOW);
 
         bool continuityPass = testNet(netPins[net]);
         bool shortsPass = testOtherNets(net);
 
         netPass[net] = continuityPass && shortsPass;
         updateCircle(net, netPass[net]);
 
         if (!netPass[net]) {
             Serial.printf("Net %d failed:\n", net);
             if (!continuityPass) Serial.println("  - Continuity test failed");
             if (!shortsPass) Serial.println("  - Short detected with another net");
         }
 
         mcp.pinMode(netPins[net][0], INPUT_PULLUP);
     }
 
     delay(100);
 }
 
 // --------- Layout Switching and Setup ---------
 void selectLayout(int layoutIndex) {
     if (layoutIndex >= 0 && layoutIndex < MAX_LAYOUTS && layoutIndex != currentLayoutIndex) {
         currentLayoutIndex = layoutIndex;
         NUM_NETS = netCounts[currentLayoutIndex];
         setupNets();
         drawLabels();
         Serial.printf("Switched to layout #%d\n", currentLayoutIndex + 1);
     }
 }
 
 void checkButtons() {
     M5.update();
     if (M5.BtnA.wasPressed()) selectLayout(0);
     if (M5.BtnB.wasPressed()) selectLayout(1);
     if (M5.BtnC.wasPressed()) selectLayout(2);
 }
 
 // --------- Net Setup ---------
 void setupNets() {
     const char* (*netConfig)[MAX_NET_SIZE + 1] = netConfigPtrs[currentLayoutIndex];
     for (int net = 0; net < NUM_NETS; net++) {
         int pinCount = 0;
         for (int i = 1; i < MAX_NET_SIZE + 1 && netConfig[net][i] != NULL; i++) {
             std::string label = netConfig[net][i];
             label.erase(0, label.find_first_not_of(" \t"));
             label.erase(label.find_last_not_of(" \t") + 1);
 
             if (pinMapping.find(label) != pinMapping.end()) {
                 netPins[net][pinCount] = pinMapping[label];
                 pinCount++;
             } else {
                 Serial.printf("Error: Unknown pin label '%s'\n", label.c_str());
             }
         }
         while (pinCount < MAX_NET_SIZE) {
             netPins[net][pinCount] = -1;
             pinCount++;
         }
     }
 
     // Set all pins as inputs with pull-ups for safety
     for (int net = 0; net < NUM_NETS; net++) {
         for (int i = 0; netPins[net][i] != -1; i++) {
             mcp.pinMode(netPins[net][i], INPUT_PULLUP);
         }
     }
 }
 
 // --------- Display Functions ---------
 void drawLabels() {
   M5.Display.fillScreen(TFT_BLACK);
   // Draw layout name at the top center
   M5.Display.drawString(layoutNames[currentLayoutIndex], M5.Display.width() / 2, 10);
   const char* (*netConfig)[MAX_NET_SIZE + 1] = netConfigPtrs[currentLayoutIndex];
   for (int i = 0; i < NUM_NETS; i++) {
       circleX[i] = startX + (i % circlesPerRow) * circleSpacingX;
       circleY[i] = startY + (i / circlesPerRow) * circleSpacingY;
       drawLabel(circleX[i], circleY[i], netConfig[i][0]);
   }
 }
 
 void updateCircle(int circleIndex, bool pass) {
     if (pass) {
         M5.Display.fillCircle(circleX[circleIndex], circleY[circleIndex], circleRadius, TFT_GREEN);
     } else {
         M5.Display.fillCircle(circleX[circleIndex], circleY[circleIndex], circleRadius, TFT_RED);
     }
 }
 
 void drawLabel(int x, int y, const char* label) {
     char line1[20], line2[20];
     char* newlinePos = strchr(label, '|');
 
     if (newlinePos) {
         strncpy(line1, label, newlinePos - label);
         line1[newlinePos - label] = '\0';
         strcpy(line2, newlinePos + 1);
 
         M5.Display.drawString(line1, x, y + circleRadius + labelSpacing);
         M5.Display.drawString(line2, x, y + circleRadius + labelSpacing + M5.Display.fontHeight());
     } else {
         M5.Display.drawString(label, x, y + circleRadius + labelSpacing);
     }
 }
 
 // --------- Test Functions ---------
 bool testNet(const int *pins) {
     bool netPass = true;
     for (int i = 1; pins[i] != -1 && i < MAX_NET_SIZE; i++) {
         int pinValue = mcp.digitalRead(pins[i]);
         if (pinValue != LOW) {
             netPass = false;
             Serial.printf("Net continuity test failed: Pin %d did not read LOW\n", pins[i]);
         }
     }
     return netPass;
 }
 
 bool testOtherNets(int currentNet) {
     bool otherNetsPass = true;
     for (int net = 0; net < NUM_NETS; net++) {
         if (net != currentNet) {
             int pinValue = mcp.digitalRead(netPins[net][0]);
             if (pinValue == LOW) {
                 otherNetsPass = false;
                 Serial.printf("Short detected: Net %d's first pin is LOW while testing Net %d\n", net, currentNet);
             }
         }
     }
     return otherNetsPass;
 }
 