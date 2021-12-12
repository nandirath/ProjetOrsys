
#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include "pitches.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// Variables
#define NOTE_SUSTAIN 1000
bool alarm=false;

WiFiClient wifiClient;
PubSubClient clientMQTT(wifiClient);
enum Sens {entree,sortie};


const unsigned char SS_PIN=D8;
const unsigned char RST_PIN=D0;
const unsigned char buzzer =D4;
const unsigned char out_push_button = D2;
const unsigned char pin_servo =D1; 
uint8_t nb_personnes=0, max_nb_personnes=10;//Decompte nombre de personnes dans la maison
int door_position=0;
volatile uint8_t enable_sortie=0;

Sens sens;

Servo porte;
String valid="63c2b702"; //Code d'entrée

// Create MFRC522 instance.
MFRC522 mfrc522(SS_PIN, RST_PIN);

//Fonctions
void  opening_door(Sens sens);
void tag_reading (String& card_content, MFRC522 card);
void gestion_personne(Sens sens);

void melody(uint8_t mel);

//Faire une fonction pour la modification du nombre maximum de personne dans la maison
//Faire une fonction pour la lecture de l'EEPROM


 void ICACHE_RAM_ATTR ISR(){
  enable_sortie=1;
}




void setup() 
{
  pinMode (buzzer,OUTPUT) ;
  pinMode (out_push_button,INPUT_PULLUP) ;
  Serial.begin(9600);
  // Initiate a serial communication
  SPI.begin();
  // Initiate  SPI bus
  mfrc522.PCD_Init();
  // Initiate MFRC522
  porte.attach(pin_servo);
  porte.write(0);
  //WiFi.begin("POEI#2", "COURS@POEI20");
  WiFi.begin("freebox_OVJEIT","kt36jt330dk6722ab");
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  reconnect();
  //melodie(2);
}

void loop() {
   if (!clientMQTT.connected()) {
    Serial.println("Disconnected...");
    reconnect();
    }
     if(analogRead(out_push_button)==LOW){
      delay(10);
      if(analogRead(out_push_button)==LOW){
        if(nb_personnes>0){
          opening_door(sortie);
          gestion_personne(sortie);
          Serial.print (" S: nombre de personnes: ");
          Serial.println (nb_personnes);
          }
         else{
          Serial.print ("Maison deja vide");
          }
        }
    }
    if ( ! mfrc522.PICC_IsNewCardPresent()) return;
    //Nvlle carte
    if ( ! mfrc522.PICC_ReadCardSerial()) return;
    //Lecture OK
    //Show UID on serial monitor
    //Serial.print("UIDtag:");
    String content= "";
    byte letter;
    tag_reading(content,mfrc522);
    delay(1000);
    if((content==valid) && (nb_personnes<=max_nb_personnes)){
        opening_door(entree);
        gestion_personne(entree);
        content="";
        Serial.print("E: nombre de personnes: ");
        Serial.println (nb_personnes);
        
         }
    else if (content!=valid) {
      char message[5];
      alarm=true;
      sprintf(message,"true");
      uint8_t count=0;
      while(alarm==true){
        count++;
      Serial.println ("Alarm");
      if (count%60==0)clientMQTT.publish("accessControl",message);
      delay(1000);
      }
      count=0;
      //melody(2);
        }
    else if (nb_personnes>=max_nb_personnes){
      
      }
}



//Lis byte par byte le tag présenté
void tag_reading (String& card_content, MFRC522 card){
  
  for (byte i = 0; i < mfrc522.uid.size; i++)   {

       // Serial.print(card.uid.uidByte[i] < 0x10 ? "0" : "");
        //Serial.print(card.uid.uidByte[i], HEX);
        card_content.concat(String(card.uid.uidByte[i] < 0x10 ?"0" : ""));
        card_content.concat(String(card.uid.uidByte[i], HEX));
        
          }
  }

//Gestion du nombre de personnne à l'interieur de la maison
void gestion_personne(Sens sens){
  if (sens==entree){
    nb_personnes++;
  }
  else if (sens==sortie) {
    nb_personnes--;
  }
  char message[3];
  sprintf(message,"%d", nb_personnes);
  clientMQTT.publish("personCount",message);
}

void opening_door(Sens sens){
  porte.write(180);
  if (sens==entree) melody(1);
  else delay(1000);
  porte.write(0);
}

void melody(uint8_t mel){
  // change this to make the song slower or faster
  if(mel==1){
    int tempo = 132;
  int melody[] = {
    NOTE_FS4,8, REST,8, NOTE_A4,8, NOTE_CS5,8, REST,8,NOTE_A4,8, REST,8, NOTE_FS4,8, //1
    NOTE_D4,8, NOTE_D4,8, NOTE_D4,8, REST,8, REST,4, REST,8, NOTE_CS4,8,NOTE_D4,8,
    NOTE_FS4,8, NOTE_A4,8, NOTE_CS5,8, REST,8, NOTE_A4,8, REST,8, NOTE_F4,8,
    NOTE_E5,-4, NOTE_DS5,8, NOTE_D5,8, REST,8, REST,4,
    };
    // sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
    // there are two values per note (pitch and duration), so for each note there are four bytes
    int notes = sizeof(melody) / sizeof(melody[0]) / 2;
    
    // this calculates the duration of a whole note in ms
    int wholenote = (60000 * 4) / tempo;
    int divider = 0, noteDuration = 0;
    for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
      // calculates the duration of each note
      divider = melody[thisNote + 1];
      if (divider > 0) {
        // regular note, just proceed
        noteDuration = (wholenote) / divider;
        }
      else if (divider < 0) {
          // dotted notes are represented with negative durations!!
          noteDuration = (wholenote) / abs(divider);
          noteDuration *= 1.5; // increases the duration in half for dotted notes
         }
      // we only play the note for 90% of the duration, leaving 10% as a pause
      tone(buzzer, melody[thisNote], noteDuration * 0.9);
      // Wait for the specief duration before playing the next note.
      delay(noteDuration);
      // stop the waveform generation before the next note.
      noTone(buzzer);
    }
    }
   else if(mel==2){
    float sinVal;
    int toneVal;
    for (int x=0; x<180; x++) {
      // convert degrees to radians then obtain sin value
      sinVal = (sin(x*(3.1412/180)));
      // generate a frequency from the sin value
      toneVal = 2000+(int(sinVal*1000));
      tone(buzzer, toneVal);
      delay(2);
      }
    }
  }
void reconnect() {
  clientMQTT.setServer("192.168.0.14", 1883);
  //clientMQTT.setServer("192.168.5.147", 1883);
  Serial.println("Trying to connect to MQTT broker");
  while (!clientMQTT.connected()) {
    if (clientMQTT.connect("Frontdoor")) ;
    else delay(1000);
    }
    Serial.println("Connected to Broker...");
}
