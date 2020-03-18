#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display


volatile int rpmcount[2] = [0, 0];
volatile int rpmcount2 = 0;
int sensorState = 0;
unsigned int rpm ;
unsigned int rpm2 ;
unsigned long timeold;
unsigned long timeold2;
//unsigned long timehold;
const long circumference = 220;
unsigned long bikespeed;
//float bikespeed;


void rpm_fun()
{
  rpmcount[0]++;
  }

void rpm_fun2()
{
  rpmcount[1]2++;
}

void setup() {

  lcd.init();                      // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();

    lcd.setCursor(3, 0);   // 3,0에서 글쓰기 시작
    lcd.print("Cadence meter");

    lcd.setCursor(3, 1);   // 3,1에서 글쓰기 시작
    lcd.print("initialized!");

    delay(800);

    lcd.clear();

    delay(500);

    lcd.setCursor(5, 1);   // 3,1에서 글쓰기 시작
    lcd.print("Cadence");                           // 케이던스 미터 초기화! 메시지 뜨고나서 케이던스 단어 프린트

  Serial.begin(115200);
  attachInterrupt(0, rpm_fun, FALLING); // 2번 핀 : 0 , 3번 핀 : 1 /// 2번 핀에 interrupt를 넣겠다.
  attachInterrupt(1, rpm_fun2, FALLING); // 각각의 핀에 해당하는 홀센서에 자석이 탐지되어서 high->low 될때마다 인터럽트되서 rpm_fun, rpm_fun2 함수 실행 -> 탐지횟수 카운트

  rpm = 0;
  rpm2 = 0;
  timeold = 0;
  timeold2 = 0;
  //timehold = 0;
}

void loop() { 

    delay(1000); // 1초마다 페달링과 뒷바퀴 rpm을 측정해서 모니터에 표시해주기 위함. 페달링 rpm은 기어변속 알고리즘의 key, 뒷바퀴 rpm은 자전거의 속도를 측정하기 위함
                 // 페달링 rpm 과 기어비를 이용해서 속도를 계산할수도 있지만, 페달링을 하지 않는 경우의 수도 고려하여 뒷바퀴 rpm을 따로 측정하기로 하였음.
                 // 하지만 이 딜레이를 넣음으로 인해 서보모터를 제어하는 코드와 합치는 과정에서 정상적으로 작동을 안하게 됨 ㅠ 나중에 코드 전체적으로 합치려면 이부분을 고쳐야할듯.



    //페달링 rpm 측정//             



    detachInterrupt(0); // rpm을 측정하는 동안에는 2번핀의 인터럽트 잠시 중단.
    
    lcd.setCursor(5, 2);
    lcd.print("   "); // LCD에 새로운 rpm값을 계속 표시해주기 위해서 이전에 표시되었던 내용을 지워주는 과정 
    
    rpm = (60000 * rpmcount[0]) / (12*(millis() - timeold)); //the 12 changes to 4 if 4 magnets are used. , 페달링 rpm 을 구하는 과정. 페달링 쪽에 자석 12개가 달려있어
                                                          //한바퀴 돌때마다 자석 12개가 찍힘. 주어진시간(millis()-timeold)동안 탐지된 자석의수 = rpmcount , 따라서 12로 나누어 주면 
                                                          //단위시간당(여기선 ms) 자전거가 몇바퀴 도는지 값이 나오고, 우리가 구하는건 round per minute이니까 단위를 맞추기 위해 
                                                          // 60000 을 곱하였음. 
    timeold = millis(); // 기준시간 리셋
    rpmcount[0] = 0;
    
    Serial.println(timeold);
    Serial.println(millis());

    //if( (millis() - timehold ) > 1000 ) { <- 앞에 있던 delay(1000)을 없애고 싶어서 이런식으로 코딩해야겠다는 생각을 했지만 그때 당시 시간과 능력부족으로.. ㅈㅅ
     
    
    lcd.clear();
    lcd.setCursor(5, 1);   // 3,1에서 글쓰기 시작
    lcd.print("Cadence");
    lcd.setCursor(5, 2);   // 5,2에서 글쓰기 시작
    lcd.print(rpm);
    lcd.setCursor(9, 2);
    lcd.print("rpm");      // lcd 모니터에 rpm 표시해주기~    , 프로토타입이라 교수들한테 보여주기용으로 만들었고 최종목적은 핸드폰 어플리케이션을 통해 핸드폰 화면을 통해서 볼수있도록 
                           // 하려고함

    //timehold = millis();
   

    attachInterrupt(0,rpm_fun,FALLING); // rpm 측정 끝났으므로 바로 다시 interrupt 넣기




    //속도측정//

    // 위에서와 같은과정, 하지만 이건 뒷바퀴(속도측정)용 //

    detachInterrupt(1); 
    
    lcd.setCursor(5, 3);
    lcd.print("   ");    //LCD에 새로운 rpm값을 계속 표시해주기 위해서 이전에 표시되었던 내용을 지워주는 과정 

    rpm2 = (60000 * rpmcount[1]]) / (8*(millis() - timeold2));  // 위에서 설명한것과 같지만 뒷바퀴에는 자석을 8개 사용함.
    bikespeed = ((rpm2 * circumference * 60 ) / 100000); // 뒷바퀴 rpm을 뒷바퀴 원주(220cm)를 이용해서 속도로 변환하는 과정.
    timeold2 = millis(); // 기준시간 리셋
    rpmcount[1] = 0;
    
    
    
    lcd.setCursor(5, 3);   // 5,2에서 글쓰기 시작
    lcd.print(bikespeed);
    lcd.setCursor(9, 3);
    lcd.print("km/h");
    
    attachInterrupt(1,rpm_fun2,FALLING);

}
