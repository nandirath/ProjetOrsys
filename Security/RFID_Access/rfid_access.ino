
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN D8
#define RST_PIN D0

 // Create MFRC522 instance.
MFRC522 mfrc522(SS_PIN, RST_PIN);
// Variables
String valid="63c2b702"; //Code d'entrée
uint8_t nb_personnes=0, max_nb_personnes=10;//Decompte nombre de personnes dans la maison
enum Sens {entree,sortie};
Sens sens;
//Fonction
void lecture_tag (String& card_content, MFRC522 card);
uint8_t gestion_personne(Sens sens);
//Faire une fonction pour la modification du nombre maximum de personne dans la maison
//Faire une fonction pour la lecture de l'EEPROM

void setup() 
{
   
    Serial.begin(9600);
    // Initiate a serial communication
    SPI.begin();
    // Initiate  SPI bus
    mfrc522.PCD_Init();
    // Initiate MFRC522
    }

void loop() {
        
    if ( ! mfrc522.PICC_IsNewCardPresent()) return;
    //Nvlle carte
    if ( ! mfrc522.PICC_ReadCardSerial()) return;
    //Lecture OK
    //Show UID on serial monitor
    Serial.print("UIDtag:");
    String content= "";
    byte letter;
    lecture_tag(content,mfrc522);
    delay(1000);
          //content.concat(String("\0"));
    if((content==valid) && gestion_personne(entree)){
        Serial.print(" entrez!");
        Serial.println (" nombre de personnes: ");
        Serial.println (nb_personnes);
         }
    else {
        Serial.print(" Val= ");
          Serial.println(content);
         }
         
}



//Lis byte par byte le tag présenté
void lecture_tag (String& card_content, MFRC522 card){
  for (byte i = 0; i < mfrc522.uid.size; i++)   {

        Serial.print(card.uid.uidByte[i] < 0x10 ? "0" : "");
        Serial.print(card.uid.uidByte[i], HEX);
        card_content.concat(String(card.uid.uidByte[i] < 0x10 ?"0" : ""));
        card_content.concat(String(card.uid.uidByte[i], HEX));
        
          }
  }
//Gestion du nombre de personnne à l'interieur de la maison
uint8_t gestion_personne(Sens sens){
  if (sens==entree){
    if(nb_personnes<=max_nb_personnes ){
    nb_personnes++;
    return 1;
  }
  else return 0;
  }
  else {
    nb_personnes--;
    return 0;
  }
  
}
