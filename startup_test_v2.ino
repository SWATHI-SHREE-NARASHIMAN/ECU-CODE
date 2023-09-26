/*****************************************************
 * Title 	: ecu_secondary.ino
 * Author   : Harish R
 * Date     : 31st December 2022
 * Purpose  : VSM ECU CODE
 * Email	: harishrajesh2002@gmail.com
 ******************************************************/

/******************************************************
 * Convention for all checks -> returns true if failure
 ******************************************************/
int debug = 1;
int flagtson=0;
int flagstarter=0;
bool TSNEW;
bool STARTERNEW;
#define SDC_RELAY 6//#define MC_POWERSTAGE 
#define FORWARD 5
#define BRAKE 10
#define RTD	 8
bool TS,STR;
#define STR_BTN 11
#define TS_BTN 12
#define SDC_IN 9
//#define TSAL_R 5

#define APPS1 A0
#define APPS2 A1
//#define CUR_SNS A3
#define BPS A3

#define BPS_THR 10
#define DIG_THR 5
#define LOOP_DELAY 10
#define LOOP_COUNT 10
bool SDC=0;
/*Analog sensor thresholds*/
#define APPS1_MIN 295
#define APPS1_MAX 480
#define APPS2_MIN 550
#define APPS2_MAX 910
#define CUR_MIN 506
#define CUR_MAX 518
#define BPS_MIN 60
#define BPS_MAX 160
#define MAX_TORQ 120

int state = 4;  // VSM state variable 
				// set state=4 for debug

bool readDigital(int pin, int thresh=DIG_THR);

void setup() {

	Serial.begin(9600);
	Serial.println("Sed lyf, but VSM works");

	pinMode(SDC_RELAY, OUTPUT);
	//pinMode(MC_POWERSTAGE, OUTPUT);
	pinMode(FORWARD, OUTPUT);
	pinMode(BRAKE, OUTPUT);
	pinMode(RTD, OUTPUT);
	
	pinMode(APPS1, INPUT_PULLUP);
	pinMode(APPS2, INPUT_PULLUP);
	pinMode(BPS, INPUT);
	//pinMode(CUR_SNS, INPUT);

	pinMode(STR_BTN, INPUT_PULLUP);
	pinMode(TS_BTN, INPUT_PULLUP);
	pinMode(SDC_IN, INPUT);
  digitalWrite(RTD, HIGH);
  digitalWrite(SDC_RELAY, LOW);
  digitalWrite(FORWARD, LOW);
  digitalWrite(BRAKE, LOW);

	// 5 relays that are controlled by ECU - all open
   TS=digitalRead(TS_BTN);
   STR=digitalRead(STR_BTN);
	//digitalWrite(MC_POWERSTAGE, LOW);
	
	delay(1000); //Buffer time for latching
	
}


void loop() {
   if(flagtson==0)
     tson();
   if(flagstarter==0 && flagtson==1 &&analogRead(BPS)>=102){
     starter();
     flagstarter==1;
   }
   bool SDCNEW;
	 Serial.println("VSM State: ");
	 Serial.println(state);
   if((SDCNEW=digitalRead(SDC_IN))==0 && SDC==1)
     digitalWrite(SDC_RELAY,LOW);
   SDC=SDCNEW;
delay(1000);
if (flagtson==1 and flagstarter==1){
  if(checkAPPS()){
	 			killTS();
	 			Serial.println("APPS Fault - Killed TS");
	 			state=0;
	}	
}

	 /*switch(state)
	 {
	 	case 0:		//LV ON

	 		Serial.println("Waiting for TS ON");
	 		if(readDigital(TS_BTN, DIG_THR)){
	 			activateTS();
	 			state=1;
	 			Serial.println("Activated TS");
	 			delay(1000);
	 		}
	 		break;

	 	case 1:		//TS ON

	 		if(checkSDC()){
	 			killTS();
	 			Serial.println("SDC Latch - Killed TS");
	 			state=0;
	 		}
	 	if(readDigital(TS_BTN, DIG_THR)){
	 			killTS();
	 			state=0;
	 			Serial.println("Inactivated TS");
	 			delay(1000);
	 		}
	 		if(starterSignal()){
	 			startupSequence();
	 			state=2;
	 			Serial.println("Activated Startup");
	 		}

	 		break;

	 	case 2:		//RTD

	 		if(checkSDC()){
	 			killTS();
	 			Serial.println("SDC Latch - Killed TS");
	 			state=0;
	 		}
	 		
	 		if(checkBSPD()){
	 			killTS();
	 			Serial.println("BSPD Fault - Killed TS");
	 			state=0;
	 		}	
	 		break;
		
	 	default:	//DEBUG STATE

	 		pinDebug();
	 		delay(1000);
	 		Serial.println("Pin Debug");
	 }	*/

}

void tson()
{ 
  if(digitalRead(TS_BTN)!=TS)
{ flagtson=1;
  digitalWrite(SDC_RELAY, HIGH);
}
  
}

void starter()
{
 

  if(digitalRead(STR_BTN)!=STR && digitalRead(SDC_IN)==1)
	 { flagstarter=1;
    startupSequence();
   }

}

bool checkBSPD() {
	//int x = 0;
	int y = 0;
	for(int i=0; i<LOOP_COUNT; i++){
	//	x += analogRead(CUR_SNS)/LOOP_COUNT;
		y += analogRead(BPS)/LOOP_COUNT;
		delay(LOOP_DELAY);
	}
	//if(debug) Serial.println(x);
	if(debug) Serial.println(y);
	if(((y<BPS_MIN || y>BPS_MAX))) return true;
	return false;
}

bool checkAPPS() {
	int x = 0;
	int y = 0;
	for(int i=0; i<LOOP_COUNT; i++){
		x += analogRead(APPS1)/LOOP_COUNT;
		y += analogRead(APPS2)/LOOP_COUNT;
		delay(LOOP_DELAY);
	}
	x = map(x, APPS1_MIN, APPS1_MAX, 0, MAX_TORQ);
	y = map(y, APPS2_MIN, APPS2_MAX, 0, MAX_TORQ);
	Serial.println(x);
	Serial.println(y);
	if(abs(x - y) < 0.25*MAX_TORQ) return false;
	return true;
}

void killTS() {

  digitalWrite(SDC_RELAY,LOW);
}

void activateTS() {
	//Closes AIR1
	digitalWrite(SDC_RELAY, LOW);
	//delay(10);
	//Makes MC attempt precharge -> expect prech to occur 
	//digitalWrite(MC_POWERSTAGE, LOW);
	//delay(10);
}
	
bool starterSignal() {
	if(readAnalog(BPS, BPS_THR) && readDigital(STR_BTN, DIG_THR))
		return true;
	return false;
}

bool checkSDC() {
	if(readDigital(STR_BTN, DIG_THR))
		return true;
	return false;
}
void pinDebug() {
	Serial.println("APPS1 Input: ");
	Serial.println(analogRead(APPS1));
	Serial.println("APPS2 Input: ");
	Serial.println(analogRead(APPS2));
	Serial.println("Brake Pressure Input: ");
	Serial.println(analogRead(BPS));
	Serial.println("TS Button Input: ");
	Serial.println(digitalRead(TS_BTN));
	Serial.println("Starter Button Input: ");
	Serial.println(digitalRead(STR_BTN));
	Serial.println("SDC Button Input: ");
	Serial.println(digitalRead(SDC_IN));
}

void startupSequence() {
	digitalWrite(BRAKE, HIGH);
	delay(1000);
	digitalWrite(FORWARD, HIGH);
	delay(1000);
	digitalWrite(BRAKE, LOW);
	soundRTD();
	digitalWrite(RTD, HIGH);
}

void soundRTD() {
	if(debug) Serial.println("Sounding RTD");
	digitalWrite(RTD, HIGH);
	delay(150);
	digitalWrite(RTD, LOW);
	delay(150);
	digitalWrite(RTD, HIGH);
	delay(150);
	digitalWrite(RTD, LOW);
	delay(150);
	digitalWrite(RTD, HIGH);
	delay(100);
	digitalWrite(RTD, LOW);
  delay(100);
  digitalWrite(RTD,HIGH);
  delay(150);
  digitalWrite(RTD,LOW);
  delay(100);
  digitalWrite(RTD,HIGH);
  delay(100);
  digitalWrite(RTD,LOW);
  delay(100);
  digitalWrite(RTD,HIGH);
  delay(150);
  digitalWrite(RTD,LOW);
  delay(100);
  digitalWrite(RTD,HIGH);
  delay(100);
  digitalWrite(RTD,LOW);
  delay(100);
  

}

bool readDigital(int pin, int thresh=DIG_THR)
{
	int x = 0;
	for(int i=0; i<LOOP_COUNT; i++){
		x += digitalRead(pin);
		delay(LOOP_DELAY);
	}
	if(debug) Serial.println(x);
	if(x < thresh) return true;
	return false;
}

bool readAnalog(int pin, int thresh)
{
	int x = 0;
	for(int i=0; i<LOOP_COUNT; i++){
		x += analogRead(pin)/LOOP_COUNT;
		delay(LOOP_DELAY);
	}
	if(debug) Serial.println(x);
	if(x < thresh) return true;
	return false;
}
