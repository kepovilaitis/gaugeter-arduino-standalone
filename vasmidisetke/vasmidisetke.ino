#include <Nextion.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <dht11.h>

// Master RX, TX, connect to Nextion TX, RX
SoftwareSerial nexSerial(0,1);

#define ONE_WIRE_BUS_PIN 2
#define DHT11_PIN 3

OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);

dht11 DHT11;

struct Warning {
  float oilWarningVal;
  float chargeWarningVal;
  float coolantTempWarningVal;
};

int oilPressureIndex = 0;
int chargeIndex = 1;
int coolantTempIndex = 2;
int outsideTempIndex = 3;

const int outTempPageNr = 0;
const int paramsPageNr = 1;
const int settingsPageNr = 2;

int oilPressurePicId = 11;
int oilPressureWarnPicId = 1;

int chargePicId = 10;
int chargeWarnPicId = 1;

int coolantTempPicId = 12;
int coolantTempWarnPicId = 2;

// Pages
NexPage outTempPage = NexPage(outTempPageNr, 0, "page0");
NexPage paramsPage = NexPage(paramsPageNr, 0, "page1");
NexPage settingsPage = NexPage(settingsPageNr, 0, "page2");

NexButton nextPageBtnPg0 = NexButton(outTempPageNr, 3, "btnNextPage");
NexButton nextPageBtnPg1 = NexButton(paramsPageNr, 3, "btnNextPage");
NexButton nextPageBtnPg2 = NexButton(settingsPageNr, 1, "btnNextPage");

//Page OutTemp components (Outside Temp)
NexText outTempText = NexText(outTempPageNr, 2, "tempText");

//Page Params components (All Params)
NexText coolantTempText = NexText(paramsPageNr, 1, "tempText");
NexText chargeText = NexText(paramsPageNr, 2, "chargeText");
NexText oilPressureText = NexText(paramsPageNr, 4, "oilPrText");

NexPicture oilPressurePic = NexPicture(paramsPageNr, 8, "oilPressPic");
NexPicture oilPressureWarningPic = NexPicture(paramsPageNr, 7, "oilWarnPic");
NexPicture chargePic = NexPicture(paramsPageNr, 9, "batteryPic");
NexPicture chargeWarnPic = NexPicture(paramsPageNr, 6, "batteryWarnPic");
NexPicture coolantTempPic = NexPicture(paramsPageNr, 10, "tempPic");
NexPicture coolantTempWarnPic = NexPicture(paramsPageNr, 5, "tempWarnPic");

//Page Settings components (Settings)
NexButton chargeMinusBtn = NexButton(settingsPageNr, 8, "chargeMinusBtn");
NexButton chargePlusBtn = NexButton(settingsPageNr, 5, "chargePlusBtn");

NexButton coolantMinusBtn = NexButton(settingsPageNr, 9, "waterMinusBtn");
NexButton coolantPlusBtn = NexButton(settingsPageNr, 6, "waterPlusBtn");

NexButton oilMinusBtn = NexButton(settingsPageNr, 10, "oilMinusBtn");
NexButton oilPlusBtn = NexButton(settingsPageNr, 7, "oilPlusBtn");

NexText chargeLimitText = NexText(settingsPageNr, 12, "chargeLmtNr");
NexText coolantLimitText = NexText(settingsPageNr, 13, "waterLmtNr");
NexText oilLimitText = NexText(settingsPageNr, 14, "oilLmtNr");

int pageNumber = outTempPageNr;

float sensorValues[4] = {0.00f, 0.00f, 0.00f, 0.00f};

double oilPressurePin = A0;  
double chargePin = A1;
double coolantTempPin = A2;
double outsideTempPin = A3;

Warning warning = {0.00f, 0.00f, 0.00f};

char degreeSymbol[6] = " C\xB0";

NexTouch *nex_Listen_List[] = {
  &nextPageBtnPg0,
  &nextPageBtnPg1,
  &nextPageBtnPg2,
  &chargeMinusBtn,
  &chargePlusBtn,
  &coolantMinusBtn,
  &coolantPlusBtn,
  &oilMinusBtn,
  &oilPlusBtn,
  NULL
}; 

bool isEngineRunning = false;

void setup() {  // Put your setup code here, to run once:

  nexSerial.begin(9600);
  sensors.begin();

  getParams();

  nextPageBtnPg0.attachPop(nextPagePopCAllback, &nextPageBtnPg0);
  nextPageBtnPg1.attachPop(nextPagePopCAllback, &nextPageBtnPg1);
  nextPageBtnPg2.attachPop(nextPagePopCAllback, &nextPageBtnPg2);

  chargeMinusBtn.attachPop(chargeWarningMinusPopCallback, &chargeMinusBtn);
  chargePlusBtn.attachPop(chargeWarningPlusPopCallback, &chargePlusBtn);
  
  coolantMinusBtn.attachPop(coolantTempWarningMinusPopCallback, &coolantMinusBtn);
  coolantPlusBtn.attachPop(coolantTempWarningPlusPopCallback, &coolantPlusBtn);
  
  oilMinusBtn.attachPop(oilWarningMinusPopCallback, &oilMinusBtn);
  oilPlusBtn.attachPop(oilWarningPlusPopCallback, &oilPlusBtn);

  readEEPROM();

  nexInit();
  changePage(pageNumber);
}  // End of setup

// Put your main code here, to run repeatedly:
void loop() {  

  nexLoop(nex_Listen_List);
  getParams();

  if(isWarningRequired()){
    pageNumber = paramsPageNr;
    changePage(pageNumber);
  }

  setUpPageText(pageNumber);

  delay(500);
}  // End of loop

void changePage(int number){

  switch(number){
    case outTempPageNr:
      outTempPage.show();
      break;
    case paramsPageNr:
      paramsPage.show();
      break;
    case settingsPageNr:    
      settingsPage.show();
      break;
  }
}

void setUpPageText(int number){

  switch(number){
    case outTempPageNr:
      setOutTempPageTextValues();
      break;
    case paramsPageNr:
      setParamsPageTextValues();
      break;
    case settingsPageNr:    
      setSettingsPageTextValues();
      break;
  }
}

void getParams(){

  sensors.requestTemperatures(); 
  DHT11.read(DHT11_PIN);

  sensorValues[oilPressureIndex] = analogRead(oilPressurePin);//sensors.getTempCByIndex(0);// Oil Pressure
  sensorValues[chargeIndex] = analogRead(chargePin);//sensors.getTempCByIndex(0);// ChargePin
  sensorValues[coolantTempIndex] = (float)sensors.getTempCByIndex(0) * 4;// Water Temp 
  sensorValues[outsideTempIndex] = (float)DHT11.temperature;// Outside Temp


  //Oil                                                               mapping in 10 and dividing it for more accurate reading
  sensorValues[oilPressureIndex] = map (sensorValues[0], 0, 1023, 0, 70) / 10.0f;

  //Charge
  sensorValues[chargeIndex] = map (sensorValues[1], 0, 1023, 0, 160) / 10.0f;
}

bool isWarningRequired(){
  return (/*sensorValues[oilPressureIndex] < warning.oilWarningVal 
      || sensorValues[chargeIndex] < warning.chargeWarningVal 
      || */sensorValues[coolantTempIndex] > warning.coolantTempWarningVal) 
    && pageNumber != 1;
}

void setOutTempPageTextValues(){

  char charVal[5];

  dtostrf(sensorValues[outsideTempIndex], 3, 1, charVal);
  outTempText.setText(strcat(charVal, degreeSymbol));
}

void setParamsPageTextValues(){
  char charVal[8];

  setParamPicture(oilPressureIndex, warning.oilWarningVal, oilPressurePic, oilPressureWarningPic, oilPressurePicId, oilPressureWarnPicId);

  dtostrf(sensorValues[oilPressureIndex], 3, 1, charVal);
  oilPressureText.setText(strcat(charVal, " bar"));

  setParamPicture(chargeIndex, warning.chargeWarningVal, chargePic, chargeWarnPic, chargePicId, chargeWarnPicId);

  dtostrf(sensorValues[chargeIndex], 3, 1, charVal);
  chargeText.setText(strcat(charVal, " V"));

  setParamPicture(coolantTempIndex, warning.coolantTempWarningVal, coolantTempPic, coolantTempWarnPic, coolantTempPicId, coolantTempWarnPicId);

  dtostrf(sensorValues[coolantTempIndex], 3, 1, charVal);
  coolantTempText.setText(strcat(charVal, degreeSymbol));
}

void setSettingsPageTextValues(){
  char value[6];

  dtostrf((warning.chargeWarningVal / 10 ), 3, 1, value);
  chargeLimitText.setText(value);

  dtostrf(warning.coolantTempWarningVal, 3, 1, value);
  coolantLimitText.setText(value);

  dtostrf((warning.oilWarningVal / 10), 3, 1, value);
  oilLimitText.setText(value);
}

void setParamPicture(int index, float warningValue, NexPicture pictureObject, NexPicture warningPictureObject, int pictureId, int warningPictureId){
  if (sensorValues[index] > warning.coolantTempWarningVal)
  {
    pictureObject.setPic(13);
    warningPictureObject.setPic(warningPictureId);
  } else {
    pictureObject.setPic(pictureId);
    warningPictureObject.setPic(13);
  }
}

void readEEPROM(){

  warning.chargeWarningVal = EEPROM.read(0);
  warning.coolantTempWarningVal = EEPROM.read(1);
  warning.oilWarningVal = EEPROM.read(2);
}

void nextPagePopCAllback(void *ptr){
  
  if (pageNumber >= 0 && pageNumber < 2){
    pageNumber = pageNumber + 1;
  } else {
    pageNumber = 0;
  }

  changePage(pageNumber);
}

void chargeWarningMinusPopCallback(void *ptr){
  onClick(0, NexText(2, 12, "chargeLmtNr"), warning.chargeWarningVal -= 1.0f);
}

void chargeWarningPlusPopCallback(void *ptr){
  onClick(0, NexText(2, 12, "chargeLmtNr"), warning.chargeWarningVal += 1.0f);
}

void coolantTempWarningMinusPopCallback(void *ptr){
  onClick(1, NexText(2, 13, "waterLmtNr"), warning.coolantTempWarningVal -= 1.0f);
}

void coolantTempWarningPlusPopCallback(void *ptr){
  onClick(1, NexText(2, 13, "waterLmtNr"), warning.coolantTempWarningVal += 1.0f);
}

void oilWarningMinusPopCallback(void *ptr){
  onClick(2, NexText(2, 14, "oilLmtNr"), warning.oilWarningVal -= 1.0f);
}

void oilWarningPlusPopCallback(void *ptr){
  onClick(2, NexText(2, 14, "oilLmtNr"), warning.oilWarningVal += 1.0f);
}

void onClick(uint8_t myByte, NexText numberText, float warningValue){

  char value[6];
  dtostrf(myByte == 0 || myByte == 2 ? warningValue / 10 : warningValue, 3, 1, value);
  
  numberText.setText(value);
  EEPROM.write(myByte, warningValue);
}
