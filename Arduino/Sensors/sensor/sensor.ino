#include <Wire.h>

int adr= 0x18;
uint8_t read_val[2]={0};
uint8_t processed_temp;

static const uint8_t luxPin= A0;
uint16_t rawlux=0;

void read_temperature();
uint8_t process_temperature();
uint8_t send_value;
byte sent,c;


void setup (){
    Serial.begin(9600);
    Wire.begin();
    pinMode(MISO, OUTPUT);
    pinMode(SCK, INPUT);
    // turn on SPI in slave mode
    SPCR |= 1<<6;
    // turn on interrupts
    SPCR |=1<<7;
}
void loop(){
    rawlux= analogRead(luxPin); //Valeure brute sur 10bits
    Serial.print("Lum: ");
    Serial.println(rawlux);
    read_temperature();
    processed_temp=process_temperature();
    Serial.print("Temp: ");
    Serial.println(processed_temp);
    delay(400);
}
ISR (SPI_STC_vect) // SPI interrupt routine
{
    c = SPDR;
    switch(c){
      case 01:
      send_value=rawlux>>8;
      break;
      case 02:
      send_value=rawlux& 0xFF;
      break;
      case 03:
      send_value= processed_temp;
      
      break;
    }
    SPDR=send_value;
} 
//Fonctions sensors management
void read_temperature(){
    Wire.beginTransmission(adr);
    Wire.write(0x05);
    Wire.endTransmission();
    Wire.requestFrom(adr,2);
    if(2 <= Wire.available()){
        read_val[1]= Wire.read();
        read_val[0]= Wire.read();
      }
}

uint8_t process_temperature(){
    //Full value :
    uint16_t res = (read_val[1]<<8)|read_val[0];
    short temperature ; 
    //Bit signe : if == 1 -> valeur négative
    uint16_t sign_bit = res & (0x800); 
    if(sign_bit == 1){
        //extension du bit de signe
        res = res|(0xE000);
        }
    else{
        temperature = res & (0xFFF);
        }  
    float val_final = 0.0625 * temperature;
    return (uint8_t) val_final;
}
