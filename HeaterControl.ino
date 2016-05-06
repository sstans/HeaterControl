/*
 RTD Sampler and Heater Control
 By: Scott Stansberry
     
     This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.
     http://creativecommons.org/licenses/by-sa/4.0/
     
     Partially derived from public domain tutorial(s) code found at arduino website: 
     http://arduino.cc/en/Tutorial/AnalogInput
 
 The sense circuit:
 
 3V -----------|
               |
               \
               /  1kohm
               \
               /
               |
               |
 A0 -----------|
               |
               \
               / 100ohm RTD (e.g. Minco S651PDY24B adhered 
               \  to device under test or nearby.)
               /
               |
               |
 GND -----------
 
 * 1 kohm pullup to 3V to Arduino A0.
 * RTD connected between Arduino input A0 and GND.
 
 The heater circuit:
   |-----------|
   |           |
   |           \  ~25ohm
   | NO1       /  4x Heaters in parallel 
   / Relay 1   \   (e.g. Omega CIR-1014/120V ~ 100ohm/heater)
   | COM1      /
   |           |
 -----         | 
  ---          |
   |   2x 20V  |
 -----         |
  ---          |
   |-----------|
 * Seeduino Relay Shield Board mounted onto Arduino Uno using spacer headers 
   (spacers opt. otherwise hardwire sense circuit above to board.)
   External 6V supply required to drive relays on this version of board.
 * 4 Omega CIR-1014/120V heaters wired in parallel between NO1 and Gnd
 * 2 20V, 3.5A surplus laptop supplies wired in series for ~40V. 
 */
 
// Hardcoded temp set point for heaters.  Change and recompile for new setpoint.
//float Setpoint = 100.0;     
float Setpoint = 125.0;     
float Deadspace = 2;      // Temp range where relay status doesn't change
float Quickheat = 10;     // Degrees C below setpoint where relay heats continously
float DutyCycle = 0.50;   // Ton/Toff
float SamplePeriod = 3000;// Sample every 5000 ms

int sensorPin = A0;   // select the A0 input pin for RTD sense.
int RelayInd = 13; // select the pin as LED indicator for the Relay control, digital output 13
int sensorValue = 0;  // variable to store the value coming from the RTD sensor
float Rnom = 100.0;
// RTD coefficients per
// http://www.rdfcorp.com/anotes/pa-rtd/pa-rtd_02.shtml

//R0	100				
//A	3.91E-003				
//B	-5.78E-007				
//C	-4.18E-012  (C=0 for t>0C)				
//	
//Rt=R0[1+A*t+B*t^2+C(t-100)*t^3]				
//T	Rt	Rlinapp	Rdiff	Tcalc	Terr
//-200	18.5	21.8	3.3	-208.5	8.5
//-190	22.8	25.7	2.9	-197.5	7.5
//-180	27.1	29.7	2.6	-186.5	6.5
//-170	31.3	33.6	2.2	-175.7	5.7
//-160	35.5	37.5	1.9	-164.9	4.9
//-150	39.7	41.4	1.7	-154.2	4.2
//-140	43.9	45.3	1.4	-143.6	3.6
//-130	48.0	49.2	1.2	-133.0	3.0
//-120	52.1	53.1	1.0	-122.5	2.5
//-110	56.2	57.0	0.8	-112.1	2.1
//-100	60.3	60.9	0.7	-101.7	1.7
//-90	64.3	64.8	0.5	-91.3	1.3
//-80	68.3	68.7	0.4	-81.0	1.0
//-70	72.3	72.6	0.3	-70.8	0.8
//-60	76.3	76.6	0.2	-60.6	0.6
//-50	80.3	80.5	0.2	-50.4	0.4
//-40	84.3	84.4	0.1	-40.2	0.2
//-30	88.2	88.3	0.1	-30.1	0.1
//-20	92.2	92.2	0.0	-20.1	0.1
//-10	96.1	96.1	0.0	-10.0	0.0
//0	100.0	100.0	0.0	0.0	0.0
//10	103.9	103.9	0.0	10.0	0.0
//20	107.8	107.8	0.0	19.9	0.1
//30	111.7	111.7	0.1	29.9	0.1
//40	115.5	115.6	0.1	39.8	0.2
//50	119.4	119.5	0.1	49.6	0.4
//60	123.2	123.4	0.2	59.5	0.5
//70	127.1	127.4	0.3	69.3	0.7
//80	130.9	131.3	0.4	79.1	0.9
//90	134.7	135.2	0.5	88.8	1.2
//100	138.5	139.1	0.6	98.5	1.5
//110	142.3	143.0	0.7	108.2	1.8
//120	146.1	146.9	0.8	117.9	2.1
//130	149.8	150.8	1.0	127.5	2.5
//140	153.6	154.7	1.1	137.1	2.9
//150	157.3	158.6	1.3	146.7	3.3
//160	161.1	162.5	1.5	156.2	3.8
//170	164.8	166.4	1.7	165.7	4.3
//180	168.5	170.3	1.9	175.2	4.8

//Linear approximation is sufficient from 0c to ~140C where error is <3C
//Rt =  R0(1+A*t)

double a = 3.9083 * pow(10,-3);  
//double b = -5.775 * pow(10, -7);  
//double c = -4.183 * pow(10,-12);

float Rpu = 985.0; // Measured.  Nom 1k
float Rrtd = 0;    // Calculated R value of RTD
float Vout =0;     // Calculated voltage on A0
float Vquant = 1.1/1024;  // Steps in A0 A/D
float T = 0;              // Temp variable
float Vs = 3.31;          // Vs as measured, Nom 3.3V
float Ton = 0;            // Relay on time
float Toff = 0;           // Relay off time
unsigned long time;       // elapsed uptime stamp
unsigned long mstos = 1000; // divisor to convert to secs

unsigned char relayPin[4] = {
  4,5,6,7};

void setup() {
  analogReference(INTERNAL);
  Serial.begin(115200);
  int i;
  // Setup arduino to control relay inputs RP0 = Relay 1 control pin, RP1 = Relay 2, etc.
  // "HIGH" excites relay coil switching from Normal position to opposite switch position.
  // declare the RelayInd as OUTPUT to drive LED indicator (HIGH is lit = Relay control excited).
  pinMode(RelayInd, OUTPUT);
  for(i = 0; i < 4; i++)
  {
    pinMode(relayPin[i],OUTPUT); //RP0 = D7, RP1 = D6, RP2 = D5, RP3 = D4 (Dn is Arduino digital pin)
    digitalWrite(relayPin[i],LOW);
  }
  Ton = DutyCycle * SamplePeriod;
  Toff = SamplePeriod - Ton;
  if (Toff < 500 ) Toff = 0;  //Due to comm delays 0.5 s and shorter Toffs don't work.
  Serial.print(Ton);
  Serial.print(" - ");
  Serial.print(Toff);
  Serial.print("\r\n");
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);    
  Vout = sensorValue * Vquant;
  Rrtd = -Rpu * Vout / (Vout - Vs);        // Solve voltage divider
  T = (Rrtd - Rnom) / (Rnom * a);          // Rt =  R0(1+A*t) Algebra ensues
  
  // turn the RelayInd on
  //digitalWrite(RelayInd, HIGH);  
  Serial.print("\r\n\n ----- Up Time(secs): ");
  Serial.print(millis()/mstos);
  Serial.print("\r\n*******************\r\nSensor value = ");
  Serial.print(sensorValue);  
  Serial.print("\r\nTemperature = ");
  Serial.print(T);  
  Serial.print("C  ");
  Serial.print(9*T/5+32);                  // Convert to Fahrenheit
  Serial.print(" F\r\n");
  //delay(2000);

  // Logic below assumes relay is normally open ie. heater off.
  if (T < (Setpoint - Quickheat))          // Temp is in Quickheat region, turn on heater
  {
    Serial.print("\r\nQuickheat.");
//    Serial.print(Ton);
    digitalWrite(RelayInd, HIGH);  
    digitalWrite(relayPin[3], HIGH);
    delay(Ton+Toff);
//    if (Toff != 0) {
//      Serial.print(" --  Roff = ");
//      Serial.print(Toff);
//      digitalWrite(RelayInd, HIGH);  
//      digitalWrite(relayPin[3], LOW);
//      delay(Toff);
//    }
  }
  else if (T > (Setpoint + Deadspace))     // Temp above setpoint, turn off heater
  {
    Serial.print("\r\nNear Setpoint");
    digitalWrite(RelayInd, LOW); 
    digitalWrite(relayPin[3], LOW);
    delay(Ton+Toff);
  }
  else                                     // Temp in control region near setpoint. turn on heater
  {                                        // for Ton ms
    Serial.print("\r\nControlled. Ron = ");
    Serial.print(Ton);
    digitalWrite(RelayInd, HIGH);  
    digitalWrite(relayPin[3], HIGH);
    delay(Ton);
    Serial.print(" --  Roff = ");
    Serial.print(Toff);
    digitalWrite(RelayInd, LOW);  
    digitalWrite(relayPin[3], LOW);
    delay(Toff);
  }
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.println("0,0,0");   // send an initial string
    delay(300);
  }
}

