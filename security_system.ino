#define alarm 12 // alarm pin

#define x A0 // X axis measurement analog pin 0
#define y A1 // Y axis measurement analog pin 1
#define z A2 // Z axis measurement analog pin 2

#define echo_ 2 // records the distance utilizing the pulse input
#define trig_ 3 //triggers the pulse

long duration; // sound wave distance
int distance; // distance in cm


int XAverage=0;
int YAverage=0;
int ZAverage=0;
 
/*Macros*/
#define trials 50 // To decrease bias in the readings we use this variable to help us get the average of the previous positions
#define max_displacement 10 // max displacement threshold in cm
#define min_displacement -10 // min displacement threshold in cm
#define ultraThreshold 150 // threshold for the ultrasonic sensor
 
void setup()
{

  pinMode(trig_, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echo_, INPUT); // Sets the echoPin as an INPUT
  Serial.begin(9600); // Starting the serial monitor

  delay(2000);

  pinMode(alarm, OUTPUT);
  digitalWrite(alarm, HIGH);

  for(int i=0;i<trials;i++) // Reading the displacement of the code in all of the trials
  {
    XAverage+=analogRead(x);
    YAverage+=analogRead(y);
    ZAverage+=analogRead(z);
  }
 
  XAverage/=trials; // taking avg for x
  YAverage/=trials; // taking avg for y
  ZAverage/=trials; // taking avg for z

}
 
void loop()
{
  int currXPos=analogRead(x); // Reading the current position of X
  int currYPos=analogRead(y); // Reading the current position of Y
  int currZPos=analogRead(z); // Reading the current position of Z
 
  int xDisplacement=XAverage-currXPos; // displacement in x
  int yDisplacement=YAverage-currYPos; // displacement in y
  int zDisplacement=ZAverage-currZPos; // displacement in z

  digitalWrite(trig_, LOW);
  delayMicroseconds(2);
  // the trigpin starts to send the pulses to the entity
  digitalWrite(trig_, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_, LOW);
  // Reads the time of the travel of the sound wave with the use of echo
  duration = pulseIn(echo_, HIGH);
  // distance is calculated in cm
  distance = duration * 0.034 / 2; // This is all divided by 2 to get the distance in cm

  if((xDisplacement < min_displacement || xDisplacement > max_displacement || yDisplacement < min_displacement || yDisplacement > max_displacement || zDisplacement < min_displacement || zDisplacement > max_displacement) && distance < ultraThreshold)
  {
    /*
        Triggers the alarm when both the thresholds are breached
    */
    digitalWrite(alarm, LOW); 
    delay(1000);
    digitalWrite(alarm, HIGH); 
    delay(1000);
    /* Prints the ultrasonic distance and the accelerometer displacements for x, y, and z 
    This only will display if the both the thresholds are breached */
    Serial.println("Intrusion Detected");
    Serial.print("x=");
    Serial.println(xDisplacement);
    Serial.print("y=");
    Serial.println(yDisplacement);
    Serial.print("z=");
    Serial.println(zDisplacement);
    Serial.println("#");
    Serial.print("Distance: ");
    Serial.println(distance);
  }
}