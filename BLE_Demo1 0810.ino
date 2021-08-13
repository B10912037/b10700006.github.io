#include <bluefruit.h>
#define BUF_LENGTH          4
#define DEVICE_NAME       "SimpleBLEPeripheral"
#define TICK_INTERVAL_us    (6*1000)

/* ECG Service: 00001523-1212-EFDE-1523-785FEABCD123
 * ECG RAW : 00001524-1212-EFDE-1523-785FEABCD123
 */
const uint8_t HS_UUID_SERVICE[] =
{
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0xF0, 0xFF, 0x00, 0x00
};
const uint8_t HS_UUID_CHR_RAW[] =
{
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0xF1, 0xFF, 0x00, 0x00
};

static uint16_t adc_value[BUF_LENGTH] = {0,}, temp[BUF_LENGTH] = {0,};
static uint8_t channels[8] = {A2, A2, A3, A3, A4, A4, A5, A5};
static int counter = 0, ch = 0, i = 0, count = 0, NUM[4] = {0,1,2,3};
int LED[10]={~0x3F, ~0x06, ~0x5B, ~0x4F, ~0x66, ~0x6D, ~0x7D, ~0x07, ~0x7F, ~0x6F};
int LED_ON[4]={0x01, 0x02, 0x04, 0x08};
int LED_ag[7]={24, 25, 26, 5, 22, 23, 13};
int LED_ch[4]={33, 2, 0, 1};
static bool connected2 = false;

BLEDis  bledis;    // DIS (Device Information Service) helper class instance
BLEService        hs(HS_UUID_SERVICE);
BLECharacteristic hsraw(HS_UUID_CHR_RAW);

extern "C"
{
  void SysTick_Handler(void)
  {
     if (connected2){
      ch=(ch+1) &3; //ch:0...3
      hsraw.read(temp, BUF_LENGTH);
      LED7();
      if (++counter >= 4) {
        adc_value[1] = analogRead(channels[0]);
        adc_value[0] = temp[0];
        hsraw.notify(adc_value, BUF_LENGTH);
        counter = 0;
      }
    }
    else{
      LEDch(0);
    }
  }
} // extern C

void LED7()
{
  count=temp[0];
  NUM[0]=count%10;
	NUM[1]=(count/10)%10;
	NUM[2]=(count/100)%10;
	NUM[3]=count/1000; //取count進陣列
  LEDnum(0x7F);
  LEDch(LED_ON[ch]);
  LEDnum(LED[NUM[ch]]);
}
void LEDch(int n)
{
  int bit=0;
  for(i=0;i<4;i++){
    bit=(n>>i)&1;
    if(bit==0){
      digitalWrite(LED_ch[i], LOW); //LED就亮了
    }
    else{
      digitalWrite(LED_ch[i], HIGH); //LED就亮了
    }
  }
}
void LEDnum(int n)
{
  int bit=0;
  for(i=0;i<7;i++){
    bit=(n>>i)&1;
    if(bit==0){
      digitalWrite(LED_ag[i], LOW); //LED就亮了
    }
    else{
      digitalWrite(LED_ag[i], HIGH); //LED就亮了
    }
  }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(33, OUTPUT);
  Serial.begin(115200);
  Serial.println("Bluefruit52 nRF Blinky Example");
  Serial.println("------------------------------\n");
  analogReadResolution(12);

  Serial.println("Initialise the Bluefruit nRF52 module");
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);  
  Bluefruit.begin();
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.setTxPower(4);

  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  Bluefruit.Periph.setConnInterval(6, 12);

  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("Yutech, Taiwan");
  bledis.setModel("TriAnswer");
  bledis.begin();

  hs.begin();
  hsraw.setProperties(CHR_PROPS_WRITE_WO_RESP | CHR_PROPS_WRITE | CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  hsraw.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  hsraw.setMaxLen(BUF_LENGTH);
  hsraw.setFixedLen(BUF_LENGTH);
  hsraw.begin();
  hsraw.write(temp, BUF_LENGTH);
  Serial.println("Setting up the advertising");
  startAdv();
  SysTick_Config( (F_CPU/1000000)*TICK_INTERVAL_us );
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(hs);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop()
{
  //Do nothing
}

void connect_callback(uint16_t conn_handle)
{
  BLEConnection* conn = Bluefruit.Connection(conn_handle);
  Serial.println("Connected");
  delay(1000);
  connected2 = true;
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;
  connected2 = false;
}
