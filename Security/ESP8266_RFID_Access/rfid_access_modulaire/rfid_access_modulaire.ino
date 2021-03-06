

#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include "pitches.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>


// Variables
#define NOTE_SUSTAIN 1000
uint8_t alarm=0;
WiFiClient wifiClient;
PubSubClient clientMQTT(wifiClient);
enum Sens {entree,sortie};


const unsigned char SS_PIN=D8;
const unsigned char RST_PIN=D0;
const unsigned char buzzer =D4;
const unsigned char out_push_button = D1;
const unsigned char pin_servo =D2; 
uint8_t nb_personnes=0, max_nb_personnes=4;//Decompte nombre de personnes dans la maison
int door_position=0;
volatile uint8_t enable_sortie=0;
char rMessage[10];

int count=0;
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


ICACHE_RAM_ATTR void ISR(){
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
  
  WiFi.begin("POEI#2", "COURS@POEI20");
  //WiFi.begin("freebox_OVJEIT","kt36jt330dk6722ab");
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  reconnect();
  attachInterrupt(digitalPinToInterrupt(out_push_button), ISR,RISING);
  //melodie(2);
}

void loop() {
  clientMQTT.loop();
  /*count++;
  /*if(count=134){
    Serial.println (rMessage);
  Serial.print ("Alarm");
  Serial.println (alarm);
  Serial.print("Condition");
  Serial.println(rMessage=="false");
  count=0;
  }*/
  if(strcmp (rMessage,"false")==0){
    alarm=0;
    Serial.println("Desactivé");
    memset(rMessage, 0, sizeof(rMessage));
  }
   if (!clientMQTT.connected()) {
    Serial.println("Disconnected...");
    reconnect();
    }
      if(enable_sortie){ 
        if (alarm==0){
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
        enable_sortie=0;
      }
    if ( ! mfrc522.PICC_IsNewCardPresent()) return;
    //Nvlle carte
    if ( ! mfrc522.PICC_ReadCardSerial()) return;
    //Lecture OK
    //Show UID on serial monitor
    String content= "";
    byte letter;
    tag_reading(content,mfrc522);
    delay(1000);
    if((content==valid) && (nb_personnes<max_nb_personnes)){
      if(!alarm){
        opening_door(entree);
        gestion_personne(entree);
        content="";
        Serial.print("E: nombre de personnes: ");
        Serial.println (nb_personnes);
        }
         }
    else if (content!=valid) {
      alarm=1;
      char* sMessage = "true";
      Serial.println ("Alarm");
      Serial.println (alarm);
      clientMQTT.publish("accessControl",sMessage);
      delay(1000);
      melody(2);
        }
    else if (nb_personnes>=max_nb_personnes){}
    
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
  Serial.println("Ouverture");
  porte.write(180);
  if (sens==entree) melody(1);
  else delay(1000);
  delay(1000);
  Serial.println("Ouvert2");
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
    for(int tours=0; tours<5;tours++){
      Serial.println("Alamr son");
      for (int x=0; x<180; x++) {
        // convert degrees to radians then obtain sin value
        sinVal = (sin(x*(3.1412/180)));
        // generate a frequency from the sin value
        toneVal = 2000+(int(sinVal*1000));
        tone(buzzer, toneVal);
        delay(2);
      }
    }
    noTone(buzzer);
    }
  }
  
void reconnect() {
  //clientMQTT.setServer("192.168.0.10", 1883);
  clientMQTT.setServer("192.168.5.147", 1883);
  Serial.println("Trying to connect to MQTT broker");
  while (!clientMQTT.connected()) {
    if (clientMQTT.connect("Frontdoor")) ;
    else delay(1000);
    }
    Serial.println("Connected to Broker...");
    clientMQTT.subscribe("accessControl"); // Subscribe to a topic
    clientMQTT.setCallback(MQTTcallback); // Set the callback function
    
}
void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("length : ");
  memset(rMessage, 0, sizeof(rMessage));
  strncpy(rMessage, (char*)payload, length);
  Serial.println(length);
  Serial.print("Received message : ");
  Serial.println(rMessage);
 
  
  
}
