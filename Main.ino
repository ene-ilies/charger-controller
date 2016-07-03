#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

const int ANALOG_VOLTAGE_REF = 5;
const int ANALOG_MAX_VALUE = 1024;
const double INPUT_SOURCE_MAX_VOLTAGE = 27;
const double INPUT_MAX_VOLTAGE = 4.5;
const double BATT_SOURCE_MAX_VOLTAGE = 12;
const double BATT_MAX_VOLTAGE = 2;
const double BATT_DISCHARGE_THRESHOLD = 1;

int led = 8;
int battVoltagePin = A3;
int inputVoltagePin = A2;
int loadControllerPin = 9;
int dummyLoadControllerPin = 10;
int chargingControllerPin = 11;

LiquidCrystal_I2C lcd(0x3f,16,2);  // set the LCD address to 0x27 for a 20 chars and 4 line display

void initDisplay()
{
	// initialize display
	lcd.init();
 
	// turn backlight on
	lcd.backlight();
	lcd.setCursor(0, 0);
	lcd.print("I:");
	lcd.setCursor(9, 0);
	lcd.print("B:");
}

double computePercentFromValue(double value, double percent) {
	return (percent * value) / 100;
}

void displayInputVoltage(double inputFullPercent) {
	double inputVoltage = computePercentFromValue(
		INPUT_SOURCE_MAX_VOLTAGE, inputFullPercent);
	lcd.setCursor(2, 0);
	lcd.print("     ");
	lcd.setCursor(2, 0);
	lcd.print(inputVoltage);
}

void displayBattVoltage(double battFullPercent) {
	double battVoltage = computePercentFromValue(
		BATT_SOURCE_MAX_VOLTAGE, battFullPercent);
	lcd.setCursor(11, 0);
	lcd.print("     ");
	lcd.setCursor(11, 0);
	lcd.print(battVoltage);
}

void delayOneSecond() {
	for (int i = 0; i < 100; i++) {
		delay(10);
	}
}

double analogAVGReadFromNProbes(int inputPin, int noOfProbes) {
	double sum = 0;
	for (int i = 0; i < noOfProbes; i++) {
		sum += analogRead(inputPin);
	}
	return sum / noOfProbes;
}

double readVoltageForNProbes(int inputPin, int noOfProbes) {
	double avgVoltage = analogAVGReadFromNProbes(inputPin, noOfProbes);
	double readVoltage = (ANALOG_VOLTAGE_REF * avgVoltage) 
		/ ANALOG_MAX_VALUE;

	if (readVoltage < 0) {
		readVoltage = 0;
	}
	
	return readVoltage;
}

void updateLoadControllerPinValue(double battVoltage) {
	char loadDutyCycle = 0;
	if (battVoltage > BATT_DISCHARGE_THRESHOLD) {
		loadDutyCycle = 255;
	}
	analogWrite(loadControllerPin, loadDutyCycle);
}

double computePercent(double value, double maxValue) {
	double percent = (value * 100) / maxValue;
	
	if (percent > 100) {
		percent = 100;
	}

	return percent;
}

byte computeDutyCycleBasedOnPercent(double percent) {
	byte dutyCycle = 0;
	if (percent > 0) {
		dutyCycle = computePercentFromValue(255, percent);
	}
	return dutyCycle;
} 

void updateBattAndDummyLoadControllerPinValue(double battFullPercent) {
	byte dummyLoadCtrlDutyCycle = computeDutyCycleBasedOnPercent(
		battFullPercent);
	byte battCtrlDutyCycle = ~dummyLoadCtrlDutyCycle;
	lcd.setCursor(0, 1);
	lcd.print("      ");
	lcd.setCursor(0, 1);
	lcd.print(battFullPercent);
	lcd.setCursor(9, 1);
	lcd.print("   ");
	lcd.setCursor(9, 1);
	lcd.print(battCtrlDutyCycle);
	lcd.setCursor(13, 1);
	lcd.print("   ");
	lcd.setCursor(13, 1);
	lcd.print(dummyLoadCtrlDutyCycle);
	analogWrite(chargingControllerPin, battCtrlDutyCycle);
	analogWrite(dummyLoadControllerPin, dummyLoadCtrlDutyCycle);
}

void setChargerPinsMode() {
	pinMode(led, OUTPUT);
	pinMode(loadControllerPin, OUTPUT);
	pinMode(dummyLoadControllerPin, OUTPUT);
	pinMode(chargingControllerPin, OUTPUT);
	pinMode(inputVoltagePin, INPUT);
	pinMode(battVoltagePin, INPUT);
}

void setup() {
	setChargerPinsMode();	
	initDisplay();
}

void loop()
{
	digitalWrite(led, HIGH);
	delayOneSecond();
	double inputVoltage = readVoltageForNProbes(inputVoltagePin, 150);
	double inputFullPercent = computePercent(
		inputVoltage, INPUT_MAX_VOLTAGE);
	displayInputVoltage(inputFullPercent);
	double battVoltage = readVoltageForNProbes(battVoltagePin, 150);
	double battFullPercent = computePercent(battVoltage, BATT_MAX_VOLTAGE);
	displayBattVoltage(battFullPercent);
	updateLoadControllerPinValue(battVoltage);
	updateBattAndDummyLoadControllerPinValue(battFullPercent);
	digitalWrite(led, LOW);
	delayOneSecond();
}
