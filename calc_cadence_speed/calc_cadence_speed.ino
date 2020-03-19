#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display


volatile int rpmcount[2] = {0, 0};
unsigned int rpm[2] = {0, 0};
unsigned long timeold[2] = {0, 0};
const long circumference = 220;
unsigned long bikespeed;


void rpm_fun1() {// 인터럽트로 카운트 증가하는 함수
  rpmcount[0]++;
}

void rpm_fun2() {
  rpmcount[1]++;
}
void (*rpm_fun[2])() = {rpm_fun1, rpm_fun2};    // int형 반환값, int형 매개변수 두 개가 있는 함수 포인터 배열 선언, 첫 번째 요소에 함수의 메모리 주소 저장

void rpm_calc();
void lcd_init();;
void setup() {
  Serial.begin(115200);
  lcd_init();
  for (int i = 0; i < 2; i++) {
    attachInterrupt(i, rpm_fun[i], FALLING); // 인터럽트 0->2번핀 1->3번핀 홀센서에 자석이 들어오고 나갈떄 FALLING이 일어나게 됨
  }

}

void loop() {
  for (int i = 0; i < 2; i++) {
    detachInterrupt(i);
    rpm_calc[i];
    attachInterrupt(i, rpm_fun[i], FALLING); // 인터럽트 0->2번핀 1->3번핀
  }

  lcd.setCursor(5, 2);
  lcd.print("   "); // LCD에 새로운 rpm값을 계속 표시해주기 위해서 이전에 표시되었던 내용을 지워주는 과정

  rpm[0] = (60000 * rpmcount[0]) / (12 * (millis() - timeold[0])); //the 12 changes to 4 if 4 magnets are used. , 페달링 rpm 을 구하는 과정. 페달링 쪽에 자석 12개가 달려있어
  //한바퀴 돌때마다 자석 12개가 찍힘. 주어진시간(millis()-timeold)동안 탐지된 자석의수 = rpmcount , 따라서 12로 나누어 주면
  //단위시간당(여기선 ms) 자전거가 몇바퀴 도는지 값이 나오고, 우리가 구하는건 round per minute이니까 단위를 맞추기 위해
  // 60000 을 곱하였음.
  timeold[0] = millis(); // 기준시간 리셋
  rpmcount[0] = 0;

  Serial.println(timeold[0]);
  Serial.println(millis());

  //if( (millis() - timehold ) > 1000 ) { <- 앞에 있던 delay(1000)을 없애고 싶어서 이런식으로 코딩해야겠다는 생각을 했지만 그때 당시 시간과 능력부족으로.. ㅈㅅ


  lcd.clear();
  lcd.setCursor(5, 1);   // 3,1에서 글쓰기 시작
  lcd.print("Cadence");
  lcd.setCursor(5, 2);   // 5,2에서 글쓰기 시작
  lcd.print(rpm[0]);
  lcd.setCursor(9, 2);
  lcd.print("rpm");      // lcd 모니터에 rpm 표시해주기~    , 프로토타입이라 교수들한테 보여주기용으로 만들었고 최종목적은 핸드폰 어플리케이션을 통해 핸드폰 화면을 통해서 볼수있도록
  // 하려고함

  //timehold = millis();




  //속도측정//

  // 위에서와 같은과정, 하지만 이건 뒷바퀴(속도측정)용 //


  lcd.setCursor(5, 3);
  lcd.print("   ");    //LCD에 새로운 rpm값을 계속 표시해주기 위해서 이전에 표시되었던 내용을 지워주는 과정

  rpm[0] = (60000 * rpmcount[1]) / (8 * (millis() - timeold[1])); // 위에서 설명한것과 같지만 뒷바퀴에는 자석을 8개 사용함.
  bikespeed = ((rpm[0] * circumference * 60 ) / 100000); // 뒷바퀴 rpm을 뒷바퀴 원주(220cm)를 이용해서 속도로 변환하는 과정.
  timeold[1] = millis(); // 기준시간 리셋
  rpmcount[1] = 0;



  lcd.setCursor(5, 3);   // 5,2에서 글쓰기 시작
  lcd.print(bikespeed);
  lcd.setCursor(9, 3);
  lcd.print("km/h");


}

void lcd_init() {
  lcd.init();
  lcd.backlight(); //여기 까지 기본 설정
  lcd.setCursor(3, 0); //여기서 부터 관상용
  lcd.print("Cadence meter");
  lcd.setCursor(3, 1);
  lcd.print("initialized!");
  delay(800);
  lcd.clear();
  delay(500);
  lcd.setCursor(5, 1);
  lcd.print("Cadence");
}
