
// Add a 10k restor betwwen the interrupt pins and ground.

#define COUNTER1_PIN 5
#define COUNTER2_PIN 6

volatile unsigned int counter1 = 0;
volatile unsigned int counter2 = 0;

void setup() {
  Serial.begin(9600);
  pinMode(COUNTER1_PIN, INPUT);
  pinMode(COUNTER2_PIN, INPUT);
  attachPinInterrupt(COUNTER1_PIN, myPinCallback1, HIGH);
  attachPinInterrupt(COUNTER2_PIN, myPinCallback2, HIGH);
}

void loop() {
  Serial.print(counter1);
  Serial.print(", ");
  Serial.println(counter2);
}

int myPinCallback1(uint32_t ulPin) {
  counter1++;
  return 0;
}

int myPinCallback2(uint32_t ulPin){
  counter2++;
  return 0;
}
