#include <Arduino.h>
#include <M5Unified.h>
#include <Adafruit_MCP23X17.h>
#include <map>
#include <string>

#define MCP_ADDRESS 0x27

constexpr const int MAX_GROUP_SIZE = 7;
constexpr const int MAX_MAPS = 3;
constexpr const int MAX_GROUPS = 8; // Adjust as needed

Adafruit_MCP23X17 mcp;

// -------- Pin Mapping (ONE mapping for all maps) --------
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

const char* mapNames[MAX_MAPS] = {
  "DEM w/ Power Header",
  "Plain DEM",
  "DEM w/ Pwr Hdr Tray State"
};

// -------- Group Info Arrays --------
// Map 1
const char* groupInfo1[][MAX_GROUP_SIZE + 1] = {
    {"4-7", "4", "7"},
    {"1-9-CN1|12V", "1", "9", "CN1"},
    {"2-10-CN4|5V", "2", "10", "CN4"},
    {"3-5-6-11-12|CN2-CN3 GND", "3", "5", "6", "11", "12", "CN2", "CN3"}
};

// Map 2
const char* groupInfo2[][MAX_GROUP_SIZE + 1] = {
    {"4-7", "4", "7"},
    {"5-11", "5", "11"},
    {"6-12", "6", "12"}
};

// Map 3 (example)
const char* groupInfo3[][MAX_GROUP_SIZE + 1] = {
    {"4-7", "4", "7"},
    {"5-6-11-12", "5", "6", "11", "12"}
};

// --- Array of pointers to groupInfos and their group counts ---
const char* (*groupInfoPtrs[MAX_MAPS])[MAX_GROUP_SIZE + 1] = {
    groupInfo1,
    groupInfo2,
    groupInfo3
};
const int groupCounts[MAX_MAPS] = {
    sizeof(groupInfo1) / sizeof(groupInfo1[0]),
    sizeof(groupInfo2) / sizeof(groupInfo2[0]),
    sizeof(groupInfo3) / sizeof(groupInfo3[0])
};

// --------- Globals for Current Map ---------
int currentMapIndex = 0;
int NUM_GROUPS = groupCounts[0];
int groupPins[MAX_GROUPS][MAX_GROUP_SIZE];

const int circleRadius = 20;
const int circleSpacingX = 120;
const int circleSpacingY = 100;
const int startX = 100;
const int startY = 50;
const int labelSpacing = 5;
const int labelFontSize = 2;
const int circlesPerRow = 2;

int circleX[MAX_GROUPS];
int circleY[MAX_GROUPS];

// --------- Function Prototypes ---------
void setupGroups();
bool testGroup(const int *pins);
bool testOtherGroups(int currentGroup);
void updateCircle(int circleIndex, bool pass);
void drawLabel(int x, int y, const char* label);
void drawLabels();
void selectMap(int mapIndex);
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

    setupGroups();
    drawLabels();
}

void loop() {
    checkButtons();

    bool groupPass[MAX_GROUPS];

    for (int group = 0; group < NUM_GROUPS; group++) {
        mcp.pinMode(groupPins[group][0], OUTPUT);
        mcp.digitalWrite(groupPins[group][0], LOW);

        bool continuityPass = testGroup(groupPins[group]);
        bool shortsPass = testOtherGroups(group);

        groupPass[group] = continuityPass && shortsPass;
        updateCircle(group, groupPass[group]);

        if (!groupPass[group]) {
            Serial.printf("Group %d failed:\n", group);
            if (!continuityPass) Serial.println("  - Continuity test failed");
            if (!shortsPass) Serial.println("  - Short detected with another group");
        }

        mcp.pinMode(groupPins[group][0], INPUT_PULLUP);
    }

    delay(100);
}

// --------- Map Switching and Setup ---------
void selectMap(int mapIndex) {
    if (mapIndex >= 0 && mapIndex < MAX_MAPS && mapIndex != currentMapIndex) {
        currentMapIndex = mapIndex;
        NUM_GROUPS = groupCounts[currentMapIndex];
        setupGroups();
        drawLabels();
        Serial.printf("Switched to map #%d\n", currentMapIndex + 1);
    }
}

void checkButtons() {
    M5.update();
    if (M5.BtnA.wasPressed()) selectMap(0);
    if (M5.BtnB.wasPressed()) selectMap(1);
    if (M5.BtnC.wasPressed()) selectMap(2);
}

// --------- Group Setup ---------
void setupGroups() {
    const char* (*groupInfo)[MAX_GROUP_SIZE + 1] = groupInfoPtrs[currentMapIndex];
    for (int group = 0; group < NUM_GROUPS; group++) {
        int pinCount = 0;
        for (int i = 1; i < MAX_GROUP_SIZE + 1 && groupInfo[group][i] != NULL; i++) {
            std::string label = groupInfo[group][i];
            label.erase(0, label.find_first_not_of(" \t"));
            label.erase(label.find_last_not_of(" \t") + 1);

            if (pinMapping.find(label) != pinMapping.end()) {
                groupPins[group][pinCount] = pinMapping[label];
                pinCount++;
            } else {
                Serial.printf("Error: Unknown pin label '%s'\n", label.c_str());
            }
        }
        while (pinCount < MAX_GROUP_SIZE) {
            groupPins[group][pinCount] = -1;
            pinCount++;
        }
    }

    // Set all pins as inputs with pull-ups for safety
    for (int group = 0; group < NUM_GROUPS; group++) {
        for (int i = 0; groupPins[group][i] != -1; i++) {
            mcp.pinMode(groupPins[group][i], INPUT_PULLUP);
        }
    }
}

// --------- Display Functions ---------
void drawLabels() {
  M5.Display.fillScreen(TFT_BLACK);
  // Draw map name at the top center
  M5.Display.drawString(mapNames[currentMapIndex], M5.Display.width() / 2, 10);
  const char* (*groupInfo)[MAX_GROUP_SIZE + 1] = groupInfoPtrs[currentMapIndex];
  for (int i = 0; i < NUM_GROUPS; i++) {
      circleX[i] = startX + (i % circlesPerRow) * circleSpacingX;
      circleY[i] = startY + (i / circlesPerRow) * circleSpacingY;
      drawLabel(circleX[i], circleY[i], groupInfo[i][0]);
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
bool testGroup(const int *pins) {
    bool groupPass = true;
    for (int i = 1; pins[i] != -1 && i < MAX_GROUP_SIZE; i++) {
        int pinValue = mcp.digitalRead(pins[i]);
        if (pinValue != LOW) {
            groupPass = false;
            Serial.printf("Group continuity test failed: Pin %d did not read LOW\n", pins[i]);
        }
    }
    return groupPass;
}

bool testOtherGroups(int currentGroup) {
    bool otherGroupsPass = true;
    for (int group = 0; group < NUM_GROUPS; group++) {
        if (group != currentGroup) {
            int pinValue = mcp.digitalRead(groupPins[group][0]);
            if (pinValue == LOW) {
                otherGroupsPass = false;
                Serial.printf("Short detected: Group %d's first pin is LOW while testing Group %d\n", group, currentGroup);
            }
        }
    }
    return otherGroupsPass;
}
