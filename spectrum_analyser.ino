//Declare Spectrum Shield pin connections
#define STROBE 4
#define RESET 5
#define DC_One A0
#define DC_Two A1

int freq_amp;
int Frequencies_Left[7];
int Frequencies_Right[7]; 
int i;

void setup() {
  Serial.begin(9600);

  //Set spectrum Shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);  
  digitalWrite(STROBE, HIGH);
  digitalWrite(RESET, HIGH);

  //Initialize Spectrum Analyzers
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, HIGH);
  delay(1);
  digitalWrite(STROBE, HIGH);
  delay(1);
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, LOW);
}

/*************Pull frquencies from Spectrum Shield****************/
void Read_Frequencies(){
  //Read frequencies for each band
  for (freq_amp = 0; freq_amp<7; freq_amp++)
  {
    Frequencies_Left[freq_amp] = analogRead(DC_One);
    Frequencies_Right[freq_amp] = analogRead(DC_Two); 
    digitalWrite(STROBE, HIGH);
    digitalWrite(STROBE, LOW);
  }
}

void Graph_Frequencies(){
   Serial.print("F1:   ");
   for( i= 0; i<7; i++)
   {
     Serial.print(Frequencies_Left[i]);
     Serial.print("  ");
     
   }
   Serial.println();
   Serial.print("F2:   ");
   for( i= 0; i<7; i++)
   {
     Serial.print(Frequencies_Right[i]);
     Serial.print("  ");
     
   }
   Serial.println();
}

void loop() {

  Read_Frequencies();
  Graph_Frequencies();
  delay(1500);

}
