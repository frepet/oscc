/*
 * Open SlotCar Controller
 * 
 * Arduino UNO code for a Slot Car controller.
 * 
 * @author Fredrik Peteri (fredrik@peteri.se)
 */

const bool DEBUG = true;
const long DEBUG_DELAY = 100; // In milliseconds

const int THROTTLE_LOW = 528;
const int THROTTLE_HIGH = 860;
const int BRAKE_DEAD_TIME = 10; // Delay from full brake on controller to brake in milliseconds
const int TICK_TIME = 10; // In milliseconds

const int THROTTLE_INPUT_PIN = A0;
const int THROTTLE_OUTPUT_PIN = 9;
const int FULL_THROTTLE_LED_PIN = 10;
const int FULL_BREAK_LED_PIN = 11;

int throttle = 0;
int brake = 0;
int attack = 50; 

long lastTick = millis();
long lastDebugTick = millis();
int brakeDeadTime = BRAKE_DEAD_TIME;

void setup() {
	if (DEBUG)
		Serial.begin(9600);
	pinMode(THROTTLE_INPUT_PIN, INPUT);
	pinMode(THROTTLE_OUTPUT_PIN, OUTPUT);
	pinMode(FULL_THROTTLE_LED_PIN, OUTPUT);
	pinMode(FULL_BREAK_LED_PIN, OUTPUT);
}

void loop() {
	if (millis() - lastTick > TICK_TIME) {
		throttle = getThrottle();
		
		if (throttle == 0) {
			if (brakeDeadTime <= 0) {
				throttle = 0;
				brake = 255;
				brakeDeadTime = BRAKE_DEAD_TIME;
			} else {
				brakeDeadTime--;
			}
			analogWrite(THROTTLE_OUTPUT_PIN, 0);
		} else {
			brake = 0;
			int throttle_out = map(throttle, 0, 255, attack, 255);;
			analogWrite(THROTTLE_OUTPUT_PIN, throttle_out);
		}
		
		lastTick = millis();

	}
	
	if (DEBUG && millis() - lastDebugTick > DEBUG_DELAY) {
		printDebug(throttle);
		lastDebugTick = millis();
	}

	updateLEDs();
}

int getThrottle() {
	int throttleIn = constrain(analogRead(THROTTLE_INPUT_PIN), THROTTLE_LOW, THROTTLE_HIGH);
	return map(throttleIn, THROTTLE_LOW, THROTTLE_HIGH, 0, 255);
}

void updateLEDs() {
	digitalWrite(FULL_THROTTLE_LED_PIN, throttle == 255);
	digitalWrite(FULL_BREAK_LED_PIN, brake == 255);
}

/* Prints sensor values and output values to the Serial Monitor */
void printDebug(int throttle) {
	Serial.print("Throttle IN: ");
	Serial.println(throttle);
	// Serial.print("Throttle OUT = ");
	// Serial.println(throttleOutput);
}
