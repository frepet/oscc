/*
 * Open SlotCar Controller
 * 
 * Arduino UNO code for a Slot Car controller.
 * 
 * @author Fredrik Peteri (fredrik@peteri.se)
 */

const bool DEBUG = true;
const long DEBUG_DELAY = 50; // In milliseconds

const int THROTTLE_LOW = 528;
const int THROTTLE_HIGH = 840;
const int BRAKE_DEAD_TIME = 10; // Delay from full brake on controller to brake in milliseconds
const int TICK_TIME = 10; // In milliseconds

const int THROTTLE_INPUT_PIN = A0;
const int THROTTLE_OUTPUT_PIN = 9;
const int BRAKE_ADJ_PIN = A1;
const int FULL_THROTTLE_LED_PIN = 10;
const int BRAKE_OUTPUT_PIN = 11;

int throttleIn = 0;
int throttle = 0;
int brake = 0;
int attack = 50;

int brakeAdjust = 0;

long lastTick = millis();
long lastDebugTick = millis();
int brakeDeadTime = BRAKE_DEAD_TIME;

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
}

void loop() {
	if (millis() - lastTick > TICK_TIME) {
    updatePots();
		getThrottle();
		
		if (throttleIn == 0) {
			if (brakeDeadTime <= 0) {
				safeBrake();
				brakeDeadTime = BRAKE_DEAD_TIME;
			} else {
        analogWrite(THROTTLE_OUTPUT_PIN, 0);
				brakeDeadTime--;
			}
		} else {
      safeForward();
    }
		lastTick = millis();
	}
	
	if (DEBUG && millis() - lastDebugTick > DEBUG_DELAY) {
		//printDebug();
    printBar(32, 255, throttleIn);
    printBar(32, 255, throttle);
		lastDebugTick = millis();
	}

	updateLEDs();
}

/* Set throttle pin on with PWM, but make sure that brake pin is low first */
void safeForward() {
  brake = 0;
  digitalWrite(BRAKE_OUTPUT_PIN, brake);
  throttle = map(throttleIn, 0, 255, attack, 255);
  analogWrite(THROTTLE_OUTPUT_PIN, throttle);
}

/* Set brake pin on with PWM, but make sure that throttle pin is low first */
void safeBrake() {
  throttle = 0;
  brake = brakeAdjust;
  analogWrite(THROTTLE_OUTPUT_PIN, throttle);
  analogWrite(BRAKE_OUTPUT_PIN, brake);
}

/* Very custom function to get a linear value 0-255 from the controller hall sensor */
void getThrottle() {
	int raw = constrain(analogRead(THROTTLE_INPUT_PIN), THROTTLE_LOW, THROTTLE_HIGH) - THROTTLE_LOW;
	raw = constrain(cbrt(map(raw, 0, THROTTLE_HIGH - THROTTLE_LOW, 0, 1000000)), 14, 100);
  throttleIn = map(raw, 14, 100, 0, 255);
}

/* Reads the settings potentiometers */
void updatePots() {
  brakeAdjust = map(analogRead(BRAKE_ADJ_PIN), 0, 1023, 0, 255);
}

void updateLEDs() {
	digitalWrite(FULL_THROTTLE_LED_PIN, throttle == 255);
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

