int switchPin = 12;
int ledPin = 8;
int switchState = LOW;

void setup() {
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);
    pinMode(switchPin, INPUT);
}

 // repeat infinitely
void loop() {
    switchState = digitalRead(switchPin);

    Serial.println(switchState);

    if (switchState == HIGH) {
        digitalWrite(ledPin, HIGH);
    } else {
         digitalWrite(ledPin, LOW);
     }    
 }