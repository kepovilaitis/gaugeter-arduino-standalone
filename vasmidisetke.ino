/*
Connection with Arduino Uno/Nano:
* +5V = 5V
* TX  = none
* RX  = pin 1 (TX)
* GND = GND

If you are going to use an Arduino Mega, you have to edit everything on this sketch that says "Serial"
and replace it with "Serial1" (or whatever number you are using).*/

#include <Nextion.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

// Master RX, TX, connect to Nextion TX, RX
SoftwareSerial FirstScreenSerial(0,1);
SoftwareSerial SecondScreenSerial(10,11);

struct Warning {
  float oilWarningVal;
  float chargeWarningVal;
  float waterTempWarningVal;
};

//TODO sukurti int'us kad butu is sensorvalues[0] i sensorValues[int oil = 0] 

NexPage page0 = NexPage(0, 0, "page0");
NexPage page1 = NexPage(1, 0, "page1");
NexPage page2 = NexPage(2, 0, "page2");
NexPage page3 = NexPage(3, 0, "page3");

  NexButton nextPageBtnPg0 = NexButton(0, 3, "btnNextPage");
    NexButton nextPageBtnPg1 = NexButton(1, 5, "btnNextPage");
      NexButton nextPageBtnPg2 = NexButton(2, 5, "btnNextPage");
        NexButton nextPageBtnPg3 = NexButton(3, 1, "btnNextPage");





  NexButton chargeMinusBtnPg3 = NexButton(3, 8, "chargeMinusBtn");
  NexButton chargePlusBtnPg3 = NexButton(3, 5, "chargePlusBtn");

  NexButton waterMinusBtnPg3 = NexButton(3, 9, "waterMinusBtn");
  NexButton waterPlusBtnPg3 = NexButton(3, 6, "waterPlusBtn");

  NexButton oilMinusBtnPg3 = NexButton(3, 10, "oilMinusBtn");
  NexButton oilPlusBtnPg3 = NexButton(3, 7, "oilPlusBtn");

  NexText chargeLmtNrPg3 = NexText(3, 12, "chargeLmtNr");
  NexText waterLmtNrPg3 = NexText(3, 13, "waterLmtNr");
  NexText oilLmtNrPg3 = NexText(3, 14, "oilLmtNr");

int pageNumber = 0;

double oldSensorValues[4] = {0, 0, 0, 0};
double sensorValues[4] = {0, 0, 0, 0};

double oilPressurePin = A0;  
double chargePin = A1;
double waterTempPin = A2;
double outsideTempPin = A3;

Warning warning = {0.00f, 0.00f, 0.00f};

NexTouch *nex_Listen_List[] = {
  /*NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL*/
  &nextPageBtnPg0,
  &nextPageBtnPg1,
  &nextPageBtnPg2,
  &nextPageBtnPg3,
  &chargeMinusBtnPg3,
  &chargePlusBtnPg3,
  &waterMinusBtnPg3,
  &waterPlusBtnPg3,
  &oilMinusBtnPg3,
  &oilPlusBtnPg3,
  NULL
}; 

bool isEngineRunning = false;

void setup() {  // Put your setup code here, to run once:

  Serial.begin(9600);  // Start serial comunication at baud=9600
  FirstScreenSerial.begin(9600);

  getParams();

  readEEPROM();

  isEngineRunning = (sensorValues[1] >= 133);

  nexInit();
  setUpPage(pageNumber);
}  // End of setup

// Put your main code here, to run repeatedly:
void loop() {  

  //getParams();
  nexLoop(nex_Listen_List);

  /*while(isEngineRunning){

    if (areValuesDifferent()) {
      resetValues();
      setUpPage(pageNumber);
    }

    isEngineRunning = (sensorValues[1] >= 13.3);
  }*/

  delay(100);
}  // End of loop

void setUpPage(int number){

  switch(number){
    case 0:
      setUpPage0();
    break;
    case 1:
      setUpPage1();
    case 2:    break;

      setUpPage2();
    break;
    case 3:
      setUpPage3();
    break;
  }
}

void getParams(){

  sensorValues[0] = analogRead(oilPressurePin);
  sensorValues[1] = analogRead(chargePin);
  sensorValues[2] = analogRead(waterTempPin);
  sensorValues[3] = analogRead(outsideTempPin);

  //Oil
  sensorValues[0] = map (sensorValues[0], 0, 1023, 0, 8000);
  sensorValues[0] = constrain(sensorValues[0], 0, 8000);

  //Charge
  sensorValues[1] = map (sensorValues[1], 0, 1023, 0, 15);
  sensorValues[1] = constrain(sensorValues[1], 0, 15);

  //Water Temp
  sensorValues[2] = map (sensorValues[2], 0, 1023, 0, 150);
  sensorValues[2] = constrain(sensorValues[2], 0, 150);

  //Outside Temp
  sensorValues[3] = map (sensorValues[3], 0, 1023, 0, 80);
  sensorValues[3] = constrain(sensorValues[3], 0, 80);  
}

void resetValues(){

  oldSensorValues[0] = sensorValues[0];
  oldSensorValues[1] = sensorValues[1];
  oldSensorValues[2] = sensorValues[2];
  oldSensorValues[3] = sensorValues[3];
}

void setUpPage0(){

  /*nex_Listen_List[0] = NULL;
  nex_Listen_List[1] = NULL;
  nex_Listen_List[2] = NULL;
  nex_Listen_List[3] = NULL;
  nex_Listen_List[4] = NULL;
  nex_Listen_List[5] = NULL;
  nex_Listen_List[6] = NULL;*/

  //Page 0 components
  NexText outTempTextPg0 = NexText(0, 2, "tempText");
  nextPageBtnPg0.attachPop(nextPagePopCAllback, &nextPageBtnPg0);

  nex_Listen_List[0] =  &nextPageBtnPg0;

  char charVal[5];

  page0.show();

  dtostrf(sensorValues[3], 3, 1, charVal);
  outTempTextPg0.setText(charVal);
}

void setUpPage1(){

  //nex_Listen_List[0] = NULL;
  //Page 1 components
  NexText outTempTextPg1 = NexText(1, 2, "tempText");
  NexText chargeTextPg1 = NexText(1, 4, "chargeText");
  nextPageBtnPg1.attachPop(nextPagePopCAllback, &nextPageBtnPg1);

  nex_Listen_List[0] = &nextPageBtnPg1;

  page1.show();

  char charVal[5];

  dtostrf(sensorValues[3], 3, 1, charVal);
  outTempTextPg1.setText(charVal);
  
  dtostrf(sensorValues[1], 3, 1, charVal);
  chargeTextPg1.setText(charVal);
}

void setUpPage2(){

  //nex_Listen_List[0] = NULL;

  //Page 2 components
  NexProgressBar chargeProgressBar = NexProgressBar(2, 7, "chargePrgsBar");
  NexText chargeTextPg2 = NexText(2, 8, "chargeTxt");
  NexPicture oilPressureGg  = NexPicture(2, 4, "oilPressureGg");
  NexText oilPressureTextPg2 = NexText(2, 9, "oilPressureTxt");
  NexPicture waterTempGg  = NexPicture(2, 6, "waterTempGg");
  NexText waterTempTextPg2 = NexText(2, 10, "waterTempTxt");
  NexPicture tempPicPg2  = NexPicture(2, 3, "tempPic");
  NexPicture batteryPicPg2  = NexPicture(2, 1, "batteryPic");
  NexPicture oilPicPg2  = NexPicture(2, 2, "oilPic");

  nextPageBtnPg2.attachPop(nextPagePopCAllback, &nextPageBtnPg2);

  nex_Listen_List[0] = &nextPageBtnPg2;

  page2.show();

  //temporarily holds data from vals
  char charVal[5];                

  /*int chargeTachVal = map (sensorValues[1], 0, 15, 0, 100);  
  chargeTachVal = constrain(chargeTachVal, 0, 100);  // Constrain the value so it doesn't go below or above the limits

  int oilPressureTachVal = map (sensorValues[0], 0, 8000, 0, 104);  
  oilPressureTachVal = constrain(oilPressureTachVal, 0, 104); 

  int waterTempBarVal = map (sensorValues[2], 0, 150, 0, 104);
  waterTempBarVal = constrain(waterTempBarVal, 0, 104);

  //4 is mininum width, 3 is precision; float value is copied onto buff
  dtostrf(sensorValues[1], 3, 1, charVal);
  // Send tachometer value:
  chargeProgressBar.setValue(chargeTachVal);
  chargeTextPg2.setText(charVal);

  dtostrf(sensorValues[0], 3, 1, charVal);
  oilPressureGg.setPic(oilPressureTachVal);
  oilPressureTextPg2.setText(charVal);

  dtostrf(sensorValues[2], 3, 1, charVal);
  waterTempGg.setPic(waterTempBarVal);
  waterTempTextPg2.setText(charVal);*/
}

void setUpPage3(){

  //Page 3 components

  //nex_Listen_List[0] = NULL;


  /*NexButton chargeMinusBtnPg3 = NexButton(3, 8, "chargeMinusBtn");
  NexButton chargePlusBtnPg3 = NexButton(3, 5, "chargePlusBtn");

  NexButton waterMinusBtnPg3 = NexButton(3, 9, "waterMinusBtn");
  NexButton waterPlusBtnPg3 = NexButton(3, 6, "waterPlusBtn");

  NexButton oilMinusBtnPg3 = NexButton(3, 10, "oilMinusBtn");
  NexButton oilPlusBtnPg3 = NexButton(3, 7, "oilPlusBtn");

  NexText chargeLmtNrPg3 = NexText(3, 12, "chargeLmtNr");
  NexText waterLmtNrPg3 = NexText(3, 13, "waterLmtNr");
  NexText oilLmtNrPg3 = NexText(3, 14, "oilLmtNr");*/
  
  nextPageBtnPg3.attachPop(nextPagePopCAllback, &nextPageBtnPg3);

  chargeMinusBtnPg3.attachPop(chargeWarningMinusPopCallback, &chargeMinusBtnPg3);
  chargePlusBtnPg3.attachPop(chargeWarningPlusPopCallback, &chargePlusBtnPg3);
  
  waterMinusBtnPg3.attachPop(waterTempWarningMinusPopCallback, &waterMinusBtnPg3);
  waterPlusBtnPg3.attachPop(waterTempWarningPlusPopCallback, &waterPlusBtnPg3);
  
  oilMinusBtnPg3.attachPop(oilWarningMinusPopCallback, &oilMinusBtnPg3);
  oilPlusBtnPg3.attachPop(oilWarningPlusPopCallback, &oilPlusBtnPg3);

  nex_Listen_List[0] = &nextPageBtnPg3;
  /*nex_Listen_List[1] = &chargeMinusBtnPg3;
  nex_Listen_List[2] = &chargePlusBtnPg3;
  nex_Listen_List[3] = &waterMinusBtnPg3;
  nex_Listen_List[4] = &waterPlusBtnPg3;
  nex_Listen_List[5] = &oilMinusBtnPg3;
  nex_Listen_List[6] = &oilPlusBtnPg3;*/

  page3.show();

  char value[6];
  dtostrf((warning.chargeWarningVal / 10 ), 3, 1, value);
  chargeLmtNrPg3.setText(value);

  dtostrf(warning.waterTempWarningVal, 3, 1, value);
  waterLmtNrPg3.setText(value);

  dtostrf((warning.oilWarningVal / 10), 3, 1, value);
  oilLmtNrPg3.setText(value);
}

bool areValuesDifferent(){
  
  return oldSensorValues[0] != sensorValues[0] ||  oldSensorValues[1] != sensorValues[1] ||
    oldSensorValues[2] != sensorValues[2] ||  oldSensorValues[3] != sensorValues[3];
}

void readEEPROM(){

  warning.chargeWarningVal = EEPROM.read(0);
  warning.waterTempWarningVal = EEPROM.read(1);
  warning.oilWarningVal = EEPROM.read(2);

  Serial.println(warning.chargeWarningVal, 1);
  Serial.println(warning.waterTempWarningVal, 1);
  Serial.println(warning.oilWarningVal, 1);
}


void nextPagePopCAllback(void *ptr){
  
  if (pageNumber >= 0 && pageNumber < 3){
    pageNumber += 1;
  } else {
    pageNumber = 0;
  }

  Serial.println(pageNumber);
  setUpPage(pageNumber);
}

void chargeWarningMinusPopCallback(void *ptr){
  //warning.chargeWarningVal -= 0.1f;
  Serial.println("POP");
  onClick(0, NexText(3, 12, "chargeLmtNr"), warning.chargeWarningVal -= 1.0f);
}

void chargeWarningPlusPopCallback(void *ptr){
  //warning.chargeWarningVal += 0.1f;
  Serial.println("POP");
  onClick(0, NexText(3, 12, "chargeLmtNr"), warning.chargeWarningVal += 1.0f);
}

void waterTempWarningMinusPopCallback(void *ptr){
  //warning.waterTempWarningVal -= 1.0f;
  Serial.println("POP");
  onClick(1, NexText(3, 13, "waterLmtNr"), warning.waterTempWarningVal -= 1.0f);
}

void waterTempWarningPlusPopCallback(void *ptr){
  //warning.waterTempWarningVal += 1.0f;
  Serial.println("POP");
  onClick(1, NexText(3, 13, "waterLmtNr"), warning.waterTempWarningVal += 1.0f);
}

void oilWarningMinusPopCallback(void *ptr){
  //warning.oilWarningVal -= 0.1f;
  Serial.println("POP");
  onClick(2, NexText(3, 14, "oilLmtNr"), warning.oilWarningVal -= 1.0f);
}

void oilWarningPlusPopCallback(void *ptr){
  //warning.oilWarningVal += 0.1f;
  Serial.println("POP");
  onClick(2, NexText(3, 14, "oilLmtNr"), warning.oilWarningVal += 1.0f);
}

void onClick(uint8_t myByte, NexText numberText, float warningValue){

  char value[6];
  dtostrf(myByte == 0 || myByte == 2 ? warningValue / 10 : warningValue, 3, 1, value);
  
  numberText.setText(value);
  EEPROM.write(myByte, warningValue);
}
