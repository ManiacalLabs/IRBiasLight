#include <FastLED.h>
#include <IRLib.h>
#include <EEPROM.h>
#include "global.h"

//Create a receiver object to listen on pin 2
IRrecv recv(2);

//Create a decoder object
IRdecode decoder;

//Change this to your LED count
#define numLEDs 103
CRGB _fastLEDs[numLEDs];

//IR Code Definitions. These will need to be change to use
//codes from your remote. All recieved codes are output to
//Serial. Use the monitor to find the codes of the buttons
//you want to use.
#define BRIGHT_UP	0xE0E006F9  //Remote D-Up
#define BRIGHT_DOWN	0xE0E08679	//Remote D-Down
#define COLOR_BK	0xE0E0A659	//Remote D-Left
#define COLOR_FWD	0xE0E046B9	//Remote D-Right
#define HOME		0xE0E0B44B	//Remote Exit
#define SAVE		0xE0E058A7	//Remote Menu
#define POWER		0xE0E016E9	//Remote Enter/OK
#define RAINBOW		0xE0E024DB	//Remote Sleep


//Amount to step values by on each remote press
#define BRIGHT_STEP 15
#define COLOR_STEP 5

bool powerOn = false;

void setup()
{
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	digitalWrite(3, LOW);
	digitalWrite(4, HIGH);

	Serial.begin(115200);

	readConfig();
	if(config.check != 42)
	{
		config.check = 42; //Arbitrary, used to check validity
		config.level = 65; //Defaulting to 25%
		config.color = 255 + COLOR_STEP; //Anything above 255 is white
		writeConfig();
	}

	recv.enableIRIn(); // Start the receiver

	//Change to whatever LED type you are using.
	FastLED.addLeds<WS2801, SPI_DATA, SPI_CLOCK, RGB>(_fastLEDs, numLEDs);
	FastLED.clear();
	FastLED.show();
	if(powerOn)	updateLEDs();
}

void updateLEDs()
{
	if(powerOn)
	{
		FastLED.setBrightness(config.level);
		CRGB c = CRGB::White;
		if(config.color <= 255)
		{
			hsv2rgb_rainbow(CHSV(config.color, 255, 255), c);
		}

		fill_solid(_fastLEDs, numLEDs, c);
	}
	else
	{
		FastLED.clear();
	}

	FastLED.show();
}

inline void brightUp()
{
	if(!powerOn) 
	{ 
		powerOn = true;
	}
	else
	{
		config.level += BRIGHT_STEP;
		if(config.level < BRIGHT_STEP) config.level = BRIGHT_STEP;
	}
	updateLEDs();
}

inline void brightDown()
{
	if(!powerOn) 
	{ 
		powerOn = true;
	}
	else
	{
		config.level -= BRIGHT_STEP;
		if(config.level < BRIGHT_STEP) config.level = 255;
	}
	updateLEDs();
}

inline void colorForward()
{
	if(!powerOn) 
	{ 
		powerOn = true;
	}
	else
	{
		config.color += COLOR_STEP;
		if(config.color > 255 + COLOR_STEP) config.color = 0;
	}
	updateLEDs();
}

inline void colorBack()
{
	if(!powerOn) 
	{ 
		powerOn = true;
	}
	else
	{
		config.color -= COLOR_STEP;
		if(config.color > 255 + COLOR_STEP) config.color = 255 + COLOR_STEP;
	}
	updateLEDs();
}

inline void home()
{
	powerOn = true;
	readConfig();
	updateLEDs();
}

inline void save()
{
	if(powerOn)
	{
		writeConfig();
		for(int i=0; i<2; i++)
		{
			FastLED.clear();
			FastLED.show();
			FastLED.delay(250);
			updateLEDs();
			FastLED.delay(250);
		}
	}
}

inline void power()
{
	powerOn = false;
	updateLEDs();
}

inline void rainbow()
{
	if(powerOn)
	{
		FastLED.setBrightness(config.level);
		fill_rainbow(_fastLEDs, numLEDs, 0);
		FastLED.show();
	}
}

void loop()
{
	static unsigned long _value = 0;
	//Continuously look for results. When you have them pass them to the decoder
	if (recv.GetResults(&decoder)) {
		decoder.decode();
		_value = decoder.value;
		if (_value > 0)
		{
			Serial.println(_value, HEX);
			switch(_value)
			{
				case BRIGHT_UP:
					brightUp();
					break;
				case BRIGHT_DOWN:
					brightDown();
					break;
				case COLOR_BK:
					colorBack();
					break;
				case COLOR_FWD:
					colorForward();
					break;
				case HOME:
					home();
					break;
				case SAVE:
					save();
					break;
				case POWER:
					power();
					break;
				case RAINBOW:
					rainbow();
					break;
			}
		}
		recv.resume();
	}
}

