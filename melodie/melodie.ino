/*
void setup() {
pinMode(8, OUTPUT);
}


*/

void melody(uint8_t mel);
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
