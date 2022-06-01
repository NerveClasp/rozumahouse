#define RELAY D1    // Serial data pin
#define BUTTON_1 A0    // Serial data pin
int button_1 = 0;

void setup() {
  Serial.begin(115200); // Initialize serial port for debugging.
  // put your setup code here, to run once:
  pinMode(RELAY, OUTPUT); 
  pinMode(BUTTON_1, INPUT); 
}

void loop() {
  button_1 = analogRead(BUTTON_1);
  Serial.println(button_1);
  if(button_1 > 30){
    digitalWrite(RELAY, HIGH); // sets the digital pin RELAY on
  }else{
    digitalWrite(RELAY, LOW); 
  }
}
