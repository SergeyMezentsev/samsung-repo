

// define pins
#define TX_PIN 10               // pin connected to tranciever module
#define RX_PIN  5               // pin connected to receiver module
#define LED_PRESENCE_PIN 12     //This led allows to know if all modules are present

//settings for the radioprotocol
#define RF_ERR 500              //The shortest duration of the pulse
#define NUM_OF_START_PULSES 3   //Number of the shortest durations (RF_ERR) in every pulse in the teacher's message
#define RF_START_TEACHER  5     // Number of start pulses in the teacher's message
#define RF_START_CHILD  3       //Number of start pulses in preambule of every children's message
#define MAX_NUM_OF_MODULES 1    //Maximum number of modules that will be checked

//-----------------------------------------------------------------------------------------------------
//function for teacher's message
void teacherMSGSend(void)
{
  for( uint8_t i = RF_START_TEACHER; i; i--)
  {
  digitalWrite(TX_PIN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delayMicroseconds(NUM_OF_START_PULSES*RF_ERR);
  digitalWrite(TX_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delayMicroseconds(NUM_OF_START_PULSES*RF_ERR);
  }
}
//-----------------------------------------------------------------------------------------------------
//getting coords from GPS-tracker
void GetGPSCoords(void)
{
  return;
}
//-----------------------------------------------------------------------------------------------------
//sending coords to server
void sendGPSCoords(void)
{
  return;
}

//-----------------------------------------------------------------------------------------------------
//Time period checks:
signed long timeLeft = 1600;
unsigned long currentTime = 0;

//-----------------------------------------------------------------------------------------------------
//Getting adresses from childrens' modules
uint8_t getAddr(void)
{
  
  unsigned long duration = 0;
  uint8_t counter = RF_START_CHILD;
  uint8_t flag = 1;
  do {
    duration = pulseIn(RX_PIN, HIGH, 14 * RF_ERR);
    if ((duration < (2 * RF_ERR)) || (duration > (4 * RF_ERR))) {flag = 0; break;}
  } while(--counter);

  uint8_t data = 1;
  if (flag){
    delayMicroseconds(3 * RF_ERR - 100);
    // read ADDR
    for(uint8_t i=8; i; i--) {
      duration = pulseIn(RX_PIN, HIGH, 5 * RF_ERR);
      data <<= 1;
      if (duration > (2*RF_ERR - 100)) data |= 1;
    }
  }

  if (flag)
  {
    return data;
    Serial.println("data");
  }
  else
  {
    return 0;
    Serial.println("0");
  }
}

//-----------------------------------------------------------------------------------------------------
//Checking presence of all modules
uint8_t arrPresence[32] = {0};    //If the counter here is more than 5, the module hasn't been available more than for 5 cycles
bool absentModules[32] = {0};     // If the counter is more than 5, so here we will se the flag that shows us absence of the module

void incArr()
{
  for (uint8_t i = 0; i < MAX_NUM_OF_MODULES; i++)
    if(arrPresence[i] <= 5) arrPresence[i]++;
}

bool allModulesPresent()
{
  bool presenceFlag = 1;
  for (uint8_t i = 0; i < MAX_NUM_OF_MODULES; i++)
    if (arrPresence[i] >= 5)
    {
      presenceFlag = 0;
      absentModules[i] = true;
    }

  if(presenceFlag)
    return true;
  else
    return false;
}

void printAbsentModules()
{
  Serial.println("---ALERT!---");
  Serial.println("Absent Modules:");
  for (uint8_t i = 0; i < MAX_NUM_OF_MODULES; i++)
    if(absentModules[i] == true)
    {
      Serial.print("Module:\t"); Serial.println(i+1); Serial.print("\n");
    }
}

//-----------------------------------------------------------------------------------------------------
void setup() {
  //Configuring pins
  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, LOW);

  pinMode(RX_PIN, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(LED_PRESENCE_PIN, OUTPUT);
  digitalWrite(LED_PRESENCE_PIN, LOW);

  Serial.begin(9600);
  Serial.println("Hello!");

}




void loop(){
  
  teacherMSGSend();
  delay(20);

  //increment all sites for modules in the array
  //incArr();

  //start to listen to childrens' modules
  currentTime = millis();
  while(timeLeft > 0)
  {

    //Get addresses
    uint8_t addr = 0;
    if( (addr = getAddr()) != 0){
      Serial.print("Address: "); Serial.println(addr, HEX);
      //arrPresence[addr-1] = 0;
      //absentModules[addr-1] = false;
    }
    

    //Check if the period of listening to module is out of time
    if(millis() - currentTime >= 70)
    {
      timeLeft -= (millis() - currentTime);
      currentTime = millis();
    }
  }

  /*
  //Check for presence of all modules
  if(!allModulesPresent())
  {
    printAbsentModules();
    digitalWrite(LED_PRESENCE_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PRESENCE_PIN, LOW);
  }
  */
  //Work with GPS coords
  GetGPSCoords();
  sendGPSCoords();
  timeLeft = 1600;
  delay(200);
}
