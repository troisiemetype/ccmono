void setup(){
	pinMode(31, OUTPUT);
}

void loop(){
	digitalWrite(31, HIGH);
	delay(1500);
	digitalWrite(31, LOW);
	delay(1500);
}