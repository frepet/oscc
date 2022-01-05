/*
 * Arduino on ATTiny84 code for the Open Slot Car Controller
 *
 * @author Fredrik Peteri (fredrik@peteri.se)
 */
#include <EEPROM.h>

const bool DEBUG = false;
const long DEBUG_DELAY = 50; // In milliseconds
const int TICK_TIME = 10; // In milliseconds

const int THROTTLE_INPUT_PIN = A7;
const int THROTTLE_OUTPUT_PIN = 5;
const int FULL_THROTTLE_LED_PIN = 10;
const int RESTART_LED_PIN = 8;
const int CALIBRATION_PIN = 4;
const int THROTTLE_HIGH_DEADZONE = 75;
const int THROTTLE_LOW_DEADZONE = 25;

const int BRAKE_ADJ_PIN = A0;
const int ATTACK_ADJ_PIN = A1;
const int TRACTION_ADJ_PIN = A2;
const int BRAKE_OUTPUT_PIN = 6;

/* Update an int in EEPROM */
void updateEEPROM(int addr, int val) { 
  EEPROM.update(addr, val >> 8);
  EEPROM.update(addr + 1, val & 0xFF);
}

/* Read an int from EEPROM */
int readEEPROM(int addr) {
  return (EEPROM.read(addr) << 8) + EEPROM.read(addr + 1);
}

int throttle_low = readEEPROM(0);
int throttle_high = readEEPROM(2);
int throttleIn = 0;
int throttle = 0;
int brake = 0;

int brakeAdjust = 0;
int attackAdjust = 0;
int tractionAdjust = 0;

// Settings for traction control
int tractionM = 64; // 0-255
float tractionD = (255 + tractionM) / log(255);

long startTime = millis();
long lastTick = startTime;
long lastDebugTick = startTime;

void setup() {
	if (DEBUG)
		Serial.begin(9600);
	pinMode(BRAKE_ADJ_PIN, INPUT);
	pinMode(THROTTLE_INPUT_PIN, INPUT);
	pinMode(THROTTLE_OUTPUT_PIN, OUTPUT);
	digitalWrite(THROTTLE_OUTPUT_PIN, LOW);
	pinMode(FULL_THROTTLE_LED_PIN, OUTPUT);
	pinMode(BRAKE_OUTPUT_PIN, OUTPUT);
	digitalWrite(BRAKE_OUTPUT_PIN, LOW);
  
  pinMode(CALIBRATION_PIN, INPUT_PULLUP);

	// Enable pullup on reset pin
	pinMode(11, INPUT_PULLUP);

	// Restard indicator
	pinMode(RESTART_LED_PIN, OUTPUT);
	digitalWrite(RESTART_LED_PIN, HIGH);

	// Set prescaler for TIMER1 to enable faster PWM
	TCCR1B &= ~(bit(CS10) | bit(CS11) | bit(CS12)); // Clear CS10, CS11 and CD12
	TCCR1B |= bit(CS10); // No prescaler
}

void loop() {
	if (millis() - lastTick > TICK_TIME) {
		updatePots();
		getThrottle();

		if (throttleIn == 0) {
			safeBrake();
		} else {
			safeForward();
		}
		lastTick = millis();
	}

	if (DEBUG && millis() - lastDebugTick > DEBUG_DELAY) {
		printDebug();
		printBar(32, 255, throttle);
		lastDebugTick = millis();
	}

  if (digitalRead(CALIBRATION_PIN) == LOW) {
    calibrate();
  }

	updateLEDs();
}

/* Set throttle pin on with PWM, but make sure that brake pin is low first */
void safeForward() {
	brake = 0;
	analogWrite(BRAKE_OUTPUT_PIN, brake);
	int newThrottle = map(throttleIn, 0, 255, attackAdjust, 255);
	throttle = constrain(
			min(newThrottle, throttle + exp((tractionAdjust + tractionM) / tractionD)),
			attackAdjust,
			255);
	delay(1);
	analogWrite(THROTTLE_OUTPUT_PIN, throttle);
}

/* Set brake pin on with PWM, but make sure that throttle pin is low first */
void safeBrake() {
	throttle = 0;
	analogWrite(THROTTLE_OUTPUT_PIN, throttle);
	brake = log(brakeAdjust) * 32 / log(2);
	delay(1);
	analogWrite(BRAKE_OUTPUT_PIN, brake);
}

/* Very custom function to get a linear value 0-255 from the controller hall sensor */
void getThrottle() {
	int raw = constrain(analogRead(THROTTLE_INPUT_PIN), throttle_low, throttle_high) - throttle_low;
	raw = constrain(cbrt(map(raw, 0, throttle_high - throttle_low, 0, 1000000)), 14, 100);
	throttleIn = map(raw, 14, 100, 0, 255);
}

/* Reads the settings potentiometers */
void updatePots() {
	brakeAdjust = map(analogRead(BRAKE_ADJ_PIN), 0, 1023, 0, 255);
	attackAdjust = map(analogRead(ATTACK_ADJ_PIN), 0, 1023, 0, 255);
	tractionAdjust = map(analogRead(TRACTION_ADJ_PIN), 0, 1023, 0, 255);
}

/* Updates FULL_THROTTLE_LED and RESTART_LED */
void updateLEDs() {
	digitalWrite(FULL_THROTTLE_LED_PIN, throttle == 255);

	// Turn off restart led after one second
	if (startTime + 125 < millis())
		digitalWrite(RESTART_LED_PIN, LOW);
}

/* If CALIBRATION_PIN is LOW then store new min and max values. */
void calibrate() {
  throttle_low = 1023;
  throttle_high = 0;
  int throttle_pos = analogRead(THROTTLE_INPUT_PIN);
  while (digitalRead(CALIBRATION_PIN) == LOW) {
    throttle_low = min(throttle_pos, throttle_low);
    throttle_high = max(throttle_pos, throttle_high);
    delay(10);
    throttle_pos = analogRead(THROTTLE_INPUT_PIN);
  }
  throttle_low += THROTTLE_LOW_DEADZONE;
  throttle_high -= THROTTLE_HIGH_DEADZONE;
  updateEEPROM(0, throttle_low);
  updateEEPROM(2, throttle_high);
}

/* Prints sensor values and output values to the Serial Monitor */
void printDebug() {
	Serial.print("Throttle IN: ");
	Serial.print(throttleIn);
	Serial.print(", Brake ADJ: ");
	Serial.print(brakeAdjust);
	Serial.print(", Throttle: ");
	Serial.print(throttle);
	Serial.print(", Brake: ");
	Serial.println(brake);
}

/* Prints the value x as a bar of a certain width*/
void printBar(short width, int high, int x) {
	char buff[width+1] = {'0'};
	buff[0] = '|';
	for(int i = 1; i < width-1; i++) {
		buff[i] = x > i*(high/width) ? 'X' : '-';
	}
	buff[width-1] = '|';
	buff[width] = 0;
	Serial.print(buff);
	Serial.println(x);
}
