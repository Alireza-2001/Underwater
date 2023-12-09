float battery_voltage()
{
  float Vref = 4.8; 
  float R1 = 36088; 
  float R2 = 4700; 
  float v = (analogRead(0) * Vref) / 1024.0; 
  float v2 = v / (R2 / (R1 + R2)); 
  return v2 ;
}

void setup() { 
  Serial.begin(9600);
  pinMode(A0,INPUT); 
} 
void loop() { 
  Serial.println(battery_voltage()); 
  delay(500); 
}



