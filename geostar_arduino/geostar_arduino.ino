#define GEOSTAR_BAUD 9600
#define FIFO_SIZE 1024

//struct for the ringbuffer. Size is 1024 Byte, 256 words in gostar
typedef struct
{
	int readIndex;
	int writeIndex;
	char fifo[FIFO_SIZE];
} ringbuffer_t;

String inputString = "";
boolean stringComplete = false;

void setup() {
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop() {
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
