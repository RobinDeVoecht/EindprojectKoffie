// Verbinding met de blynk app via wifi
#define BLYNK_TEMPLATE_ID "user11"
#define BLYNK_TEMPLATE_NAME "user11@server.wyns.it"
#define BLYNK_PRINT Serial
char auth[] = "RKSF8dIqYvBADvDRXX3W8oX5zQyEUm-r";
char ssid[] = "Kiekeboe";
char pass[] = "kruisstraat";
// Alle libraries die ik gebruik voor het project.
#include <ESP32Encoder.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>

//Initaliseren van alle pins. Zowel fysieke pins als virtuele pins.
#define Koffie_CLK_PIN 22
#define Koffie_DT_PIN 13
#define Suiker_CLK_PIN 5
#define Suiker_DT_PIN 4
#define LEDlamp 2
#define startKnop 14
#define VIRTUAL_PIN_KOFFIE V1
#define VIRTUAL_PIN_SUIKER V2
#define VIRTUAL_PIN_DRUKKNOP V3
#define VIRTUAL_PIN_TEXT_DISPLAY V4
#define VIRTUAL_PIN_LED V5

LiquidCrystal lcd(19, 23, 18, 17, 16, 15); //lcd scherm initialiseren

//Aanmaken van 2 verschillende encoders. 1 voor koffe en 1 voor suiker.
ESP32Encoder KoffieEncoder;
ESP32Encoder SuikerEncoder;

//variabele voor het opslaan van de huidige en de vorige waarden van de encoders.
int KoffieEncoderValue = 0;
int previousKoffieEncoderValue = 0;
int SuikerEncoderValue = 0;
int previousSuikerEncoderValue = 0;

//max en min waardes van de koffie en suiker. 
const int minKoffie = -1;
const int maxKoffie = 16;
const int minSuiker = -1;
const int maxSuiker = 16;
//Display waardes van de kofie en suiker
int koffieSterkte = 0;
int suikerSterkte = 0;
// waardes van de sliders in de blynk app.
int sliderKoffieSterkte = 0;
int sliderSuikerSterkte = 0;
//status van de drukknop
bool isButtonPressed = false;
unsigned long buttonPressedTime = 0;

//deze functie zorgt ervoor dat de rotart encoders altijd worden gelezen. De waarde van de koffie/suiker encoder zal de waarde op het lcd scherm bepalen
// ook wordt er al geprint op het lcd scherm. 
//er is een live communicatie met het lcd scherm en de encoders
void readRotaryEncoders() {
  int KoffieReading = KoffieEncoder.getCount();
  int SuikerReading = SuikerEncoder.getCount();
  
  if (KoffieReading != previousKoffieEncoderValue) {
    KoffieEncoderValue += (KoffieReading > previousKoffieEncoderValue) ? 1 : -1;
    KoffieEncoderValue = constrain(KoffieEncoderValue, minKoffie, maxKoffie);
    
    if(KoffieEncoderValue < 0){
      koffieSterkte = 0;
    } else if (KoffieEncoderValue >= 0 && KoffieEncoderValue < 4) {
      koffieSterkte = 1;
    } else if (KoffieEncoderValue >= 4 && KoffieEncoderValue < 8) {
      koffieSterkte = 2;
    } else if (KoffieEncoderValue >= 8 && KoffieEncoderValue < 12) {
      koffieSterkte = 3;
    } else if (KoffieEncoderValue >= 12 && KoffieEncoderValue <= 16) {
      koffieSterkte = 4;
    }
    lcd.setCursor(8, 0);
    lcd.print(koffieSterkte);
    previousKoffieEncoderValue = KoffieReading;
  }

  if (SuikerReading != previousSuikerEncoderValue) {
    SuikerEncoderValue += (SuikerReading > previousSuikerEncoderValue) ? 1 : -1;
    SuikerEncoderValue = constrain(SuikerEncoderValue, minSuiker, maxSuiker);
    
    if(SuikerEncoderValue < 0){
      suikerSterkte = 0;
    } else if (SuikerEncoderValue >= 0 && SuikerEncoderValue < 4) {
      suikerSterkte = 1;
    } else if (SuikerEncoderValue >= 4 && SuikerEncoderValue < 8) {
      suikerSterkte = 2;
    } else if (SuikerEncoderValue >= 8 && SuikerEncoderValue < 12) {
      suikerSterkte = 3;
    } else if (SuikerEncoderValue >= 12 && SuikerEncoderValue <= 16) {
      suikerSterkte = 4;
    }
    lcd.setCursor(8, 1);
    lcd.print(suikerSterkte);
    previousSuikerEncoderValue = SuikerReading;
  }
}

void setup() {
  Serial.begin(115200);
  // bevestig de koffie en suiker encoder pinnen
  KoffieEncoder.attachFullQuad(Koffie_CLK_PIN, Koffie_DT_PIN);
  SuikerEncoder.attachFullQuad(Suiker_CLK_PIN, Suiker_DT_PIN);
  pinMode(LEDlamp, OUTPUT);
  pinMode(startKnop, INPUT_PULLUP);
  
  //standaard tekst voor het lcd scherm + initialisatie
  lcd.begin(16, 2);
  lcd.print("Koffie: ");
  lcd.setCursor(0, 1);
  lcd.print("suiker: ");  

// verbinding maken met internet
  WiFi.begin(ssid, pass);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");

//verbinding maken met de blynk server
  Blynk.begin(auth, ssid, pass, "server.wyns.it", 8081);
}

BLYNK_WRITE(VIRTUAL_PIN_KOFFIE) {
  // in deze functie ontvang ik de sliderwaarde van de koffie en print ik die op het lcd scherm
  sliderKoffieSterkte = param.asInt();
   lcd.setCursor(8, 0);
  lcd.print(sliderKoffieSterkte);
}

BLYNK_WRITE(VIRTUAL_PIN_SUIKER) {
  //in deze functie ontvang ik de slider waarde van de suiker en print ik die op het lcd scherm
  sliderSuikerSterkte = param.asInt();
  lcd.setCursor(8, 1);
  lcd.print(sliderSuikerSterkte);
}
BLYNK_WRITE(VIRTUAL_PIN_DRUKKNOP) {
  //status van knop ontvangen
  if (param.asInt() == 1) { // Als de drukknop is ingedrukt
    startKoffieMaken(); // start van de functie startKoffieMaken
  }
}

void startKoffieMaken(){
  //In deze funtie staat alles wat er gebeurd wanneer er op de knop wordt gedrukt
  isButtonPressed = true;
  buttonPressedTime = millis();
  //maken van "uw koffie wordt klaargemaakt" zowel op lcd als in de app
  Blynk.virtualWrite(VIRTUAL_PIN_TEXT_DISPLAY," Uw koffie wordt klaargemaakt");
  Blynk.virtualWrite(VIRTUAL_PIN_LED, 255);
  digitalWrite(LEDlamp, HIGH);
  lcd.setCursor(0,0);
  lcd.print("uw koffie wordt");
  lcd.setCursor(0,1);
  lcd.print("klaargemaakt");
  //wachten van 10 seconden (simulatie van de koffiebereiding)
  delay(9500);
  //resetten van de encoderwaarden en het lcd-scherm na het bereiden vd koffie
  KoffieEncoderValue = -1;
  SuikerEncoderValue = -1;
  lcd.clear();
}

void loop() {
  // in de loop functie wordt de blynk server gestart
  Blynk.run();
  //constant nakijken van de rotary encoders door de functie op te roepen
  readRotaryEncoders();
  //indien de knop wordt, startKoffieMaken();
  if(digitalRead(startKnop)==LOW){
    startKoffieMaken();
  }
// als de knop is ingedrukt en 10 sec zijn verstreken, wordt de knop terug uitgezet en reset het scherm
  if (isButtonPressed && millis() - buttonPressedTime >= 10000) {
    isButtonPressed = false; // Zet de knop weer uit
    lcd.setCursor(0, 0);
    lcd.print("Koffie: "); // Toon de vorige inhoud op het LCD-scherm
    lcd.setCursor(0, 1);
    lcd.print("suiker: ");
    Blynk.virtualWrite(VIRTUAL_PIN_TEXT_DISPLAY, "Selecteer uw Voorkeur");
    digitalWrite(LEDlamp, LOW);
    Blynk.virtualWrite(VIRTUAL_PIN_LED, 0);
    Blynk.virtualWrite(VIRTUAL_PIN_KOFFIE, 0);   
    Blynk.virtualWrite(VIRTUAL_PIN_SUIKER, 0);    
  }
}
