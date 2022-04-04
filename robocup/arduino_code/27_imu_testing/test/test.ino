/*!
  * file readAccelData.ino
  *
  * Through the example, you can get the sensor data by using getAllData:
  * get accelerometer data of sensor.
  * 
  * With the rotation of the sensor, data changes are visible.
  *
  * Copyright   [DFRobot](http://www.dfrobot.com), 2016
  * Copyright   GNU Lesser General Public License
  *
  * version  V0.1
  * date  2019-6-25
  */

#include "DFRobot_BMX160.h"

DFRobot_BMX160 bmx160;
void setup(){
  Serial.begin(9600);
  delay(100);
  
  //init the hardware bmx160  
  if (bmx160.begin() != true){
    Serial.println("init false");
    while(1);
  }
  
  bmx160.setAccelRange(eAccelRange_8G);
  delay(100);
}

void loop(){
  bmx160SensorData  Oaccel;

  /* Get a new sensor event */
  bmx160.getAllData(NULL, NULL, &Oaccel);
  
  /* Display the accelerometer results (accelerometer data is in m/s^2) */
  Serial.print("Accel ");
  Serial.print("X: "); Serial.print(Oaccel.x    ); Serial.print("  ");
  Serial.print("Y: "); Serial.print(Oaccel.y    ); Serial.print("  ");
  Serial.print("Z: "); Serial.print(Oaccel.z    ); Serial.print("  ");
  Serial.println("m/s^2");

  Serial.println("");

  delay(500);
}
