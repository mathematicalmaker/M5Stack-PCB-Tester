#include <Arduino.h>
#include <M5Unified.h>
#include <Adafruit_MCP23X17.h>
#include <map>
#include <string>

#define MCP_ADDRESS 0x27 // MCP23017 address

// Maximum size of pin groups to be tested
constexpr const int MAX_GROUP_SIZE = 7;

Adafruit_MCP23X17 mcp;

// (Device Pin Label -> Mux Pin Number)
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

// Combined array: First element is the label for the pin group, 
// subsequent elements are device pin labels.
// Group labels get displayed on the screen, remaining elements use pinMapping to
// build the array of pin groups to be tested.
// Note: Unused slots in groupInfo are implicitly set to NULL by the compiler.
const char* groupInfo[][MAX_GROUP_SIZE + 1] = {
    {"4-7", "4", "7"},                     // Group 0
    {"1-9-CN1|12V", "1", "9", "CN1"},      // Group 1
    {"2-10-CN4|5V", "2", "10", "CN4"},     // Group 2
    {"3-5-6-11-12|CN2-CN3 GND", "3", "5", "6", "11", "12", "CN2", "CN3"} // Group 3
};

// Dynamically calculate NUM_GROUPS based on groupInfo array size
constexpr const int NUM_GROUPS = sizeof(groupInfo) / sizeof(groupInfo[0]);

// Declare groupPins array using dynamic NUM_GROUPS
int groupPins[NUM_GROUPS][MAX_GROUP_SIZE];

// Other constants and variables...
const int circleRadius = 20;
const int circleSpacingX = 120;
const int circleSpacingY = 100;
const int startX = 100;
const int startY = 50;
const int labelSpacing = 5;
const int labelFontSize = 2;
const int circlesPerRow = 2;

int circleX[NUM_GROUPS];
int circleY[NUM_GROUPS];

// Function Prototypes
void setupGroups();
bool testGroup(const int *pins);
bool testOtherGroups(int currentGroup);
void updateCircle(int circleIndex, bool pass);
void drawLabel(int x, int y, const char* label);

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Display.fillScreen(TFT_BLACK);

  if (mcp.begin_I2C(MCP_ADDRESS)) {
    Serial.println("MCP23017 found!");
  } else {
    Serial.println("MCP23017 not found.");
    M5.Display.fillScreen(TFT_RED); // Red background for error
    M5.Display.setTextColor(TFT_WHITE, TFT_RED); // White text on red background
    M5.Display.setTextDatum(MC_DATUM); // Center alignment
    M5.Display.drawString("MCP23017 NOT FOUND", M5.Display.width() / 2, M5.Display.height() / 2);    
    while (1);
  }

  // Dynamically build the groupPins array
  setupGroups();
  
  // Print out the generated groupPins array for verification
  for (int group = 0; group < NUM_GROUPS; group++) {
    Serial.printf("Group %d (%s): ", group, groupInfo[group][0]); // Print label from combined array
    for (int i = 0; i < MAX_GROUP_SIZE && groupPins[group][i] != -1; i++) {
      Serial.printf("%d ", groupPins[group][i]);
    }
    Serial.println();
  }

  // Setup pins as inputs
  for (int group = 0; group < NUM_GROUPS; group++) {
    for (int i = 0; groupPins[group][i] != -1; i++) {
      mcp.pinMode(groupPins[group][i], INPUT_PULLUP);
    }
  }

  // Configure display settings once
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK); // White text, black background
  M5.Display.setTextFont(labelFontSize);         // Set font once
  M5.Display.setTextDatum(TC_DATUM);             // Top-center alignment

  // Calculate circle positions and draw labels, circles will be drawn in loop()
  for (int i = 0; i < NUM_GROUPS; i++) {
    circleX[i] = startX + (i % circlesPerRow) * circleSpacingX;
    circleY[i] = startY + (i / circlesPerRow) * circleSpacingY;
    drawLabel(circleX[i], circleY[i], groupInfo[i][0]);
  }}

void loop() {
  bool groupPass[NUM_GROUPS];

  for (int group = 0; group < NUM_GROUPS; group++) {
    // Set the first pin of the current group LOW
    // Only the first pin of a single group is set to LOW for testing
    // The rest are set to INPUT_PULLUP (already done in setup)
    mcp.pinMode(groupPins[group][0], OUTPUT);
    mcp.digitalWrite(groupPins[group][0], LOW);

    // Test continuity within the group and check for shorts with other groups
    bool continuityPass = testGroup(groupPins[group]);
    bool shortsPass = testOtherGroups(group);

    // Combine results and update display
    groupPass[group] = continuityPass && shortsPass;
    updateCircle(group, groupPass[group]);

    // Print results to serial
    if (!groupPass[group]) {
      Serial.printf("Group %d failed:\n", group);
      if (!continuityPass) {
        Serial.println("  - Continuity test failed");
      }
      if (!shortsPass) {
        Serial.println("  - Short detected with another group");
      }
    }

    // Reset the first pin of this group back to INPUT_PULLUP
    mcp.pinMode(groupPins[group][0], INPUT_PULLUP);
  }
  
  // Delay to avoid rapid updates
  delay(100);
}

// Function Definitions
// Sets up the groupPins array based on the groupInfo array and pinMapping.
// The first element of each groupInfo entry is the label, and the rest are pin labels.
void setupGroups() {
    for (int group = 0; group < NUM_GROUPS; group++) {
        int pinCount = 0;

        // Skip the first element (label) and process pin labels
        for (int i = 1; i < MAX_GROUP_SIZE + 1 && groupInfo[group][i] != NULL; i++) {
            std::string label = std::string(groupInfo[group][i]);
            label.erase(0, label.find_first_not_of(" \t")); // Trim leading spaces
            label.erase(label.find_last_not_of(" \t") + 1); // Trim trailing spaces

            // Map the device pin label to its corresponding mux pin number
            if (pinMapping.find(label) != pinMapping.end()) {
                groupPins[group][pinCount] = pinMapping[label];
                pinCount++;
            } else {
                Serial.printf("Error: Unknown pin label '%s'\n", label.c_str());
            }
        }

        // Fill remaining slots in the array with -1 (sentinel values)
        while (pinCount < MAX_GROUP_SIZE) {
            groupPins[group][pinCount] = -1;
            pinCount++;
        }
    }
}

// Tests all pins in a group for continuity. Stops when it encounters a sentinel value (-1).
// Since the first pin is set to LOW, we check if all other pins in the group read LOW.
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

// Tests for short circuits with other groups. 
// Since the first pin of the current group is set to LOW, we check if any other group's 
// first pin is also LOW. If so, it indicates a short circuit.
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

// Updates the circle color based on the test result.
void updateCircle(int circleIndex, bool pass) {
  if (pass) {
    M5.Display.fillCircle(circleX[circleIndex], circleY[circleIndex], circleRadius, TFT_GREEN);
  } else {
    M5.Display.fillCircle(circleX[circleIndex], circleY[circleIndex], circleRadius, TFT_RED);
  }
}

// Draws the label for the group, splitting it into two lines if a '|' character is found.
void drawLabel(int x, int y, const char* label) {
  char line1[20];
  char line2[20];
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