/*
 * This sketch turns PS2 controller (via SPI interface) into a virtual PS4 gamepad using ESP32
 * Created by gvl610 (https://github.com/raspiduino) using PS2X and BleGamepad libraries.
 */

#include <BleGamepad.h>
#include <PS2X_lib.h>

// SPI pins for PS2 (controller/receiver)
#define PS2_DAT 12 // MISO
#define PS2_CMD 13 // MOSI
#define PS2_SEL 15 // SS
#define PS2_CLK 14 // SLK

// Enable/disable pressures and rumble modes
// Pressures (analog measurements of L2 and R2) is likely not available on cheap Chinese clones
// Rumble is not (yet) supported by BleGamepad, so disable it
#define pressures false
#define rumble false

// Define name and vendor of the controller
#define BLE_NAME "Wireless controller"
#define BLE_VENDOR "Sony Corp."

// Define D-Pad buttons pressed state as byte for clearer code
#define UP_BIT    0x08
#define DOWN_BIT  0x04
#define LEFT_BIT  0x02
#define RIGHT_BIT 0x01

// PS2X object
PS2X ps2x;

// BleGamepad object. You can optionally set name, manufacturer, and battery level for your gamepad
BleGamepad bleGamepad(BLE_NAME, BLE_VENDOR, 100);

// Check if in_button state is changed and update the out_button state if needed
void check_send_button(uint16_t in_button, uint8_t out_button)
{
  // Check if state changed
  if (ps2x.NewButtonState(in_button)) {
    // State changed -> check if button is currently pressed
    if (ps2x.Button(in_button)) {
      // in_button pressed -> press out_button
      bleGamepad.press(out_button);
    } else {
      // Not pressed -> release out_button
      bleGamepad.release(out_button);
    }
  }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("PS2 to BLE gamepad");

    // Init PS2X
    // Just keep trying until it's connected
    while (1) {
      Serial.print("Connecting to PS2 controller... ");

      int error = -1;
      for (int i = 0; i < 10; i++) // Trying to connect to PS2 controller. Max 10 times
      {
        delay(1000); // Wait 1 sec
        // Config pins and modes: GamePad(clock, command, attention, data, Pressures?, Rumble?)
        error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
        if (error == 0) break; // Break if connected, to save time
        Serial.print(".");
      }

      switch (error) // Check the error code
      {
      case 0:
        Serial.println(" Connected to PS2 controller");
        break;
      case 1:
        Serial.println(" Error: Cannot find controller. Check the connection");
        break;
      case 2:
        Serial.println(" Error: Cannot send command");
        break;
      case 3:
        Serial.println(" Error: Cannot enter pressures mode");
        break;
      }

      if (error == 0) break;
    }

    // Init BleGamepad
    Serial.print("Initializing BleGamepad... ");
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setAxesMin(0x00); // 0
    bleGamepadConfig.setAxesMax(0xFF); // 255
    bleGamepadConfig.setVid(0x054C); // Sony Corp.
    bleGamepadConfig.setPid(0x05C4); // DualShock 4 [CUH-ZCT1x]
    bleGamepad.begin(&bleGamepadConfig);
    Serial.println("done!");

    Serial.print("You are now ready to connect! Find the Bluetooth (low energy) device named \"");
    Serial.print(BLE_NAME);
    Serial.print("\" from vendor \"");
    Serial.print(BLE_VENDOR);
    Serial.println("\". Enjoy!");
}

void loop()
{
  // If connected -> send data
  if (bleGamepad.isConnected())
  {
    // Read gamepad data
    ps2x.read_gamepad(false, false);

    // Send axes data and L2, R2
    // Left Thumb X, Left Thumb Y, Right Thumb X, Right Thumb Y, Left Trigger, Right Trigger
    uint8_t lx = ps2x.Analog(PSS_LX);
    uint8_t ly = ps2x.Analog(PSS_LY);
    uint8_t rx = ps2x.Analog(PSS_RX);
    uint8_t ry = ps2x.Analog(PSS_RY);
    //Serial.printf("Joystick: %d,%d,%d,%d\n", lx, ly, rx, ry);
    bleGamepad.setAxes(lx, ly, rx, ry, ps2x.Button(PSB_L2) ? 255 : 0, ps2x.Button(PSB_R2) ? 255 : 0);

    // Send buttons data
    check_send_button(PSB_PINK, BUTTON_1);
    check_send_button(PSB_BLUE, BUTTON_2);
    check_send_button(PSB_RED, BUTTON_3);
    check_send_button(PSB_GREEN, BUTTON_4);
    check_send_button(PSB_L1, BUTTON_5);
    check_send_button(PSB_R1, BUTTON_6);
    // Button 6 and 7 are L2, R2 and are handled above in the axes section
    check_send_button(PSB_SELECT, BUTTON_9);
    check_send_button(PSB_START, BUTTON_10);
    check_send_button(PSB_L3, BUTTON_11);
    check_send_button(PSB_R3, BUTTON_12);

    // Send POV D-Pad data
    // If any of the D-Pad button is pressed
    if (ps2x.NewButtonState(PSB_PAD_UP) || ps2x.NewButtonState(PSB_PAD_DOWN) || ps2x.NewButtonState(PSB_PAD_LEFT) || ps2x.NewButtonState(PSB_PAD_RIGHT)) {
      // Switch case for button states
      switch((ps2x.Button(PSB_PAD_UP) ? UP_BIT : 0) |
              (ps2x.Button(PSB_PAD_DOWN) ? DOWN_BIT : 0) |
              (ps2x.Button(PSB_PAD_LEFT) ? LEFT_BIT : 0) |
              (ps2x.Button(PSB_PAD_RIGHT) ? RIGHT_BIT : 0))
      {
        case UP_BIT | LEFT_BIT:
          bleGamepad.setHat1(HAT_UP_LEFT);
          break;
        case UP_BIT | RIGHT_BIT:
          bleGamepad.setHat1(HAT_UP_RIGHT);
          break;
        case DOWN_BIT | LEFT_BIT:
          bleGamepad.setHat1(HAT_DOWN_LEFT);
          break;
        case DOWN_BIT | RIGHT_BIT:
          bleGamepad.setHat1(HAT_DOWN_RIGHT);
          break;
        case UP_BIT:
          bleGamepad.setHat1(HAT_UP);
          break;
        case DOWN_BIT:
          bleGamepad.setHat1(HAT_DOWN);
          break;
        case LEFT_BIT:
          bleGamepad.setHat1(HAT_LEFT);
          break;
        case RIGHT_BIT:
          bleGamepad.setHat1(HAT_RIGHT);
          break;
        default:
          bleGamepad.setHat1(HAT_CENTERED);
          break;
      }
    }
  }

  // Give some time for other stuffs to run, or the watchdog will complain
  delay(5);
}
