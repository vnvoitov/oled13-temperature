//
// Определение датчиков температуры и вывод на дисплей их адресов и значений
//	То же самое выводится в консоль
#include "U8glib.h"

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);	// I2C 128x64(col2-col129) SH1106,Like HeiTec 1.3' I2C OLED 

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire ds18x20[] = { 2, 2, 2, 2 };	// порт и ожидаемое количество датчиков

const int oneWireCount = sizeof(ds18x20)/sizeof(OneWire);
DallasTemperature sensor[oneWireCount];
DeviceAddress temperatureAddress[oneWireCount];
float temperatures[oneWireCount];
int  resolution = 10;
int	sensorI = 0 ;
int  delayInMillis = 0;
int  delayDisp = 1000;

uint8_t	menu_redraw_required = 0;
unsigned long uptime = 0;
unsigned long lastTempRequest = 0;
unsigned long lastDispRequest = 0;

void draw(DeviceAddress temperatureAddress[], float temperatures[], int oneWireCount, unsigned long uptime) {
  // graphic commands to redraw the complete screen should be placed here  
  //u8g.setFont(u8g_font_unifont);
  u8g.setFont(u8g_font_6x13);
  //u8g.setFont(u8g_font_osb21);
	// Печать адресов и значений температуры
	for (int k=0; k<oneWireCount; k++) {

		u8g.setPrintPos(0, 10+k*12);
	  
		for (int i=0; i<sizeof(DeviceAddress); i++) {
		    if (temperatureAddress[k][i] < 16) u8g.print("0");
			u8g.print(temperatureAddress[k][i], HEX);
		}
		u8g.print(" ");
		u8g.print(temperatures[k]);
	}
	u8g.setPrintPos(0, 10+oneWireCount*12);
	u8g.print("Uptime: ");
	u8g.print(uptime);
	
}

void setup(void) {

	digitalWrite(13, LOW);
	// start serial port
	Serial.begin(9600);
	Serial.println("Dallas Temperature Multiple Bus Control Library Simple Demo");
	Serial.print("============Ready with ");
	Serial.print(oneWireCount);
	Serial.println(" Sensors================");

	delayInMillis = 750 / (1 << (12 - resolution)); // 750ms conversion for 12 bits resolution. 93.75 ms for 9 bits
	lastTempRequest = millis();
	lastDispRequest = millis();
  
	DeviceAddress deviceAddress;
	for (int i = 0; i < oneWireCount; i++) {
		sensor[i].setOneWire(&ds18x20[i]);
		sensor[i].begin();
		if (sensor[i].getAddress(deviceAddress, 0)) {
			sensor[i].setResolution(deviceAddress, resolution);
			for (int z=0; z < sizeof(deviceAddress); z++) {
				DeviceAddress deviceAddressTMP;
				sensor[i].getAddress(deviceAddressTMP, i);
				temperatureAddress[i][z] = deviceAddressTMP[z];
			}
		}
		Serial.print(i);
		Serial.print(": ");
		printAddress(temperatureAddress[i]);
		Serial.println();
	}
	sensor[0].requestTemperatures();
	menu_redraw_required = 1;     // force initial redraw
}

void loop(void) {
 
    if (millis() - lastTempRequest >= delayInMillis) // waited long enough??
	{
		float temperature = sensor[sensorI].getTempCByIndex(sensorI);
		Serial.print("Temperature for the sensor ");
		Serial.print(sensorI);
		Serial.print(" is ");
		Serial.println(temperature);
		temperatures[sensorI] = temperature;
		sensor[sensorI].requestTemperatures();
		sensorI ++;
		if (sensorI == oneWireCount) {
			sensorI = 0 ;
		}

		lastTempRequest = millis();
	}
	
    if (millis() - lastDispRequest >= delayDisp) // waited long enough??
	{
	   // мигаем лампочкой
		digitalWrite(13, !digitalRead(13));
		
		Serial.print("Uptime: ");
		Serial.println(uptime);
		Serial.println();

		if (  menu_redraw_required != 0 ) {

			u8g.firstPage();

			do  {
				draw(temperatureAddress, temperatures, oneWireCount, uptime);
			} while( u8g.nextPage() );

			//menu_redraw_required = 0;
		}

		lastDispRequest = millis();
		uptime ++ ;
	}
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}