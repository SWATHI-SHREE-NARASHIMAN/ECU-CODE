/******************************************************
 * Convention for all checks -> returns true if failure
 ******************************************************/
//PINOUTS

//Relay connections:
#define SDC_RELAY 6
#define FORWARD 5
#define BRAKE 10
#define RTD   8
// Inputs:
#define STR_BTN 11
#define TS_BTN 12
#define SDC_IN 9
#define APPS1 A0
#define APPS2 A1
#define BPS A3
/****************************************************/
//ANALOG THRESHOLDS
#define APPS1_MIN 540
#define APPS1_MAX 1023 
#define APPS2_MIN 550
#define APPS2_MAX 800
#define CUR_MIN 506
#define CUR_MAX 518
#define BPS_MIN 60
#define BPS_MAX 160
#define MAX_TORQ 120
/***************************************************/
//FLAGS AND INITIAL PARAMETERS
int debug = 1;
int flagtson=0;
int flagstarter=0;
bool TSNEW;
bool STARTERNEW;
bool TS,STR;
bool SDC=0;
int state=4;


void setup() {

  //PINMODE DECLARATIONS
  //OUTPUTS:
  pinMode(SDC_RELAY, OUTPUT);
  pinMode(FORWARD, OUTPUT);
  pinMode(BRAKE, OUTPUT);
  pinMode(RTD, OUTPUT);
  //INPUTS:
  pinMode(APPS1, INPUT);
  pinMode(APPS2, INPUT);
  pinMode(BPS, INPUT);
  pinMode(STR_BTN, INPUT_PULLUP);
  pinMode(TS_BTN, INPUT_PULLUP);
  pinMode(SDC_IN, INPUT);
  /************************************************/
   //INITIAL STATES:
  digitalWrite(RTD, HIGH);
  digitalWrite(SDC_RELAY, LOW);
  digitalWrite(FORWARD, LOW);
  digitalWrite(BRAKE, LOW);
  /***********************************************/
  Serial.begin(9600);
  Serial.println("Serial com started");
  delay(1000); // buffer for latching

}



void loop() {

  //STARTER AND TSON CHK:
  if(flagtson==0)
     tson();
   if(flagstarter==0 && flagtson==1)
     starter();
/**********************************************/
  // RUN CHECKING FUNCTIONS CONTINUOUSLY  
   checkBSPD();
   checkAPPS();
   checkSDC();

   while(checkBSPD() || checkAPPS() || checkSDC) killTS();
 /***********************************************/
  //serial for config
  
  Serial.println("Config and Debug");
  Serial.print("APPS1: ");
  Serial.println(digitalRead(APPS1));
  Serial.print("APPS2: ");
  Serial.println(digitalRead(APPS2));
  Serial.print("BPS: ");
  Serial.println(digitalRead(BPS));
  Serial.print("SDC: ");
  Serial.println(digitalRead(SDC));
  Serial.print("TS BUTTON: ");
  Serial.println(digitalRead(TS_BTN));
  Serial.print("STARTER BUTTON: ");
  Serial.println(digitalRead(STR_BTN));
  delay(1000);

  
  
  
}






/************************************************/
//AUXILLIARY FUNCTIONS

//STARTUP FUNCTIONS IN ORDER
void tson()
{ 
  if(digitalRead(TS_BTN)!=TS)
{ flagtson=1;
  digitalWrite(SDC_RELAY, LOW);
}
}

void starter()
{
 if(digitalRead(STR_BTN)!=STR && digitalRead(SDC_IN)==1)
   { flagstarter=1;
    soundRTD();
   }
}

void soundRTD() {
  if(debug) Serial.println("Sounding RTD");
  digitalWrite(RTD, HIGH);
  delay(300);
  digitalWrite(RTD, LOW);
  delay(150);
  digitalWrite(RTD, HIGH);
  delay(300);
  digitalWrite(RTD, LOW);
  delay(150);
  digitalWrite(RTD, HIGH);
  delay(300);
  digitalWrite(RTD, LOW);
  delay(150);
  startupSequence();
}

void startupSequence() {
  digitalWrite(BRAKE, HIGH);
  delay(1000);
  digitalWrite(FORWARD, HIGH);
  delay(1000);
  digitalWrite(BRAKE, LOW);
  digitalWrite(RTD, HIGH);
}



/**********************************************/

//COMPONENTS CHK

bool checkBSPD()
{
  if(analogRead(BPS)>BPS_MIN && analogRead(BPS)<BPS_MAX) return false;
  return true;
 }


bool checkAPPS()
{
  double a1=analogRead(APPS1);
  double a2=analogRead(APPS2);

  if(abs(a1-a2)<0.25*MAX_TORQ) return false;
  return true;
}

bool checkSDC()
{
  if(digitalRead(SDC_IN)==1) return false;
  return true;
  
}

/**************************************************/

//KILL TS RESET FLAGS FOR STARTER AND TS ON BUTTON
void killTS()
{
  digitalWrite(SDC_RELAY, HIGH);
  SDC=true;
  flagtson=0;
  flagstarter=0;
}
