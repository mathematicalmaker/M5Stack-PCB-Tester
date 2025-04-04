## **PCB Testing System**

This project provides a simple and efficient system for testing PCBs (Printed Circuit Boards) using an **M5Stack Core** and an **MCP23017 I/O expander**. The system checks for continuity within groups of connected pins and detects shorts between different pin groups. It provides visual feedback on the M5Stack display and logs test results to the serial monitor.

---

## **Features**

- **Continuity Testing**: Verifies that all pins in a group are electrically connected.
- **Short Detection**: Ensures there are no shorts between different groups of pins.
- **Visual Feedback**: Displays pass/fail results for each group on the M5Stack screen using colored circles:
  - **Green Circle**: Group passed (no issues detected).
  - **Red Circle**: Group failed (continuity issue or short detected).
- **Serial Logging**: Provides detailed failure information for debugging.
- **Dynamic Configuration**: Automatically adapts to the number of pin groups defined in the configuration.

---

## **Hardware Requirements**

1. **M5Stack Core Basic** (or compatible model with a display).
2. **MCP23017 I/O Expander** (connected via I2C).
3. A PCB with pin groups to be tested.
4. Jumper wires or connectors to interface with the PCB.

---

## **Software Requirements**

1. Arduino IDE or PlatformIO.
2. Required libraries:
   - [M5Unified](https://github.com/m5stack/M5Unified)
   - [Adafruit_MCP23X17](https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library)

---

## **How It Works**

1. The program defines pin groups in the `groupInfo` array, where:
   - The first element is a label for the group (displayed on-screen).
   - Subsequent elements are device pin labels mapped to MCP23017 pins.
2. During testing:
   - Each group's first pin is set LOW, and continuity is checked for all other pins in the group.
   - Shorts are checked by ensuring no other group's first pin is LOW. (If other groups pass the first test, then inherently we only need to test one pin in each other group.)
3. Results are displayed on the M5Stack screen:
   - Green circles indicate passing groups.
   - Red circles indicate failing groups.
4. Detailed failure information is printed to the serial monitor.

---

## **Setup Instructions**

1. Connect the MCP23017 to the M5Stack via I2C:
   - Default MCP23017 address: `0x27`.
   - SDA and SCL lines connected appropriately.
2. Connect the PCB under test to the MCP23017 using jumper wires or connectors.
3. Load the code onto your M5Stack using Arduino IDE or PlatformIO.

---

## **Configuration**

### Pin Mapping

The `pinMapping` variable maps device pin labels to MCP23017 pins:

```cpp
std::map<std::string, int> pinMapping = {
    {"2", 11}, {"4", 12}, {"6", 13}, {"10", 14},
    {"12", 15}, {"1", 4}, {"3", 5}, {"5", 6},
    {"7", 7}, {"9", 8}, {"11", 9}, {"CN1", 0},
    {"CN2", 1}, {"CN3", 2}, {"CN4", 3}
};
```

### Pin Groups

Define your pin groups in `groupInfo`. Each group includes:

- A label (displayed on-screen).
- A list of device pin labels corresponding to `pinMapping`.

Example:

```cpp
const char* groupInfo[][MAX_GROUP_SIZE + 1] = {
    {"4-7", "4", "7"},                     // Group 0
    {"1-9-CN1|12V", "1", "9", "CN1"},      // Group 1
    {"2-10-CN4|5V", "2", "10", "CN4"},     // Group 2
    {"3-5-6-11-12|CN2-CN3 GND", "3", "5", "6", "11", "12", "CN2", "CN3"} // Group 3
};
```

### Display Settings

You can adjust display settings such as circle size, spacing, and font size using these constants:

```cpp
const int circleRadius = 20;
const int circleSpacingX = 120;
const int circleSpacingY = 100;
const int labelFontSize = 2;
```

---

## **Usage**

1. Power on the M5Stack with the code loaded.
2. Connect a PCB to be tested.
3. Observe test results:
   - Green circles indicate passing groups.
   - Red circles indicate failing groups.
4. Check the serial monitor for detailed failure information.

---

## **Serial Output Example**

If a failure is detected, you’ll see output like this in the serial monitor:

```
Group continuity test failed: Pin 7 did not read LOW
Group continuity test failed: Pin CN1 did not read LOW
Short detected: Group 2's first pin is LOW while testing Group 0
```

If all tests pass:

```
All groups passed!
```

---

## **Troubleshooting**

### Common Issues

1. **MCP23017 Not Found**:
   - Ensure proper I2C connections (SDA, SCL) and check the MCP23017 address (`0x27` by default).
   - If not found, an error message will appear on-screen.
2. **False Failures During Device Swaps**:
   - Brief red flashes may occur when swapping PCBs but are harmless.
3. **Incorrect Pin Mapping**:
   - Verify that `pinMapping` matches your hardware configuration.

---

## **Contributing**

Feel free to submit issues or pull requests to improve this project!

---

## **License**

This project is licensed under the MIT License.

---

Let me know if you'd like me to refine any section or add more details! 😊
