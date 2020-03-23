#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

volatile int rpmcount[2] = {0, 0};
const float circumference = 2.2; //바퀴둘레 : 2.2mm
const int hall_num[2] = {12, 8}; //홀센서 갯수
unsigned long lcd_update_before = 0;
const int lcd_update_interval = 300; // 300ms마다 업데이트
unsigned long rpm_update_before = 0;
const int rpm_update_interval = 1000; //1000ms마다 업데이트
float rpm_arr[2] = {0.0, 0.0};

void rpm_fun1() {// 인터럽트로 카운트 증가하는 함수
  rpmcount[0]++;
}
void rpm_fun2() {
  rpmcount[1]++;
}

void rpm_calc();
void lcd_init();
void lcd_update(float rpm[]);

void setup() {
  Serial.begin(115200);
  lcd_init();
  for (int i = 0; i < 2; i++) {
    attachInterrupt(i, rpm_fun[i], FALLING); // 인터럽트 0->2번핀 1->3번핀 홀센서에 자석이 들어오고 나갈떄 FALLING이 일어나게 됨
  }
}

void loop() {
  for (int i = 0; i < 2; i++) {
    rpm_calc(i);
  }
  lcd_update(rpm_arr);
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
}

void lcd_update(float rpm[]) {
  if ((millis() - lcd_update_before) >= lcd_update_interval) {
    lcd_update_before = millis();
    lcd.setCursor(5, 1);   // 3,1에서 글쓰기 시작
    lcd.print("Cadence");
    Serial.print("rpm : ");
    lcd.setCursor(5, 2);
    lcd.print("   "); // LCD에 새로운 rpm값을 계속 표시해주기 위해서 이전에 표시되었던 내용을 지워주는 과정
    lcd.setCursor(5, 2);   // 5,2에서 글쓰기 시작
    lcd.print((int)rpm[0]);
    Serial.println(rpm[0]);
    lcd.setCursor(9, 2);
    lcd.print("rpm");      // lcd 모니터에 rpm 표시해주기
    Serial.print("speed : ");
    lcd.setCursor(5, 3);
    lcd.print("    ");    //LCD에 새로운 rpm값을 계속 표시해주기 위해서 이전에 표시되었던 내용을 지워주는 과정
    lcd.setCursor(5, 3);   // 5,3에서 글쓰기 시작
    float bikespeed = ((rpm[1] * circumference * 60 ) /1000);
    lcd.print((int)bikespeed);
    Serial.println(bikespeed);
    lcd.setCursor(9, 3);
    lcd.print("km/h");
  }
}

void rpm_calc() {
  if ( (millis() - rpm_update_before) >= rpm_update_interval ) {
    detachInterrupt(0);
    detachInterrupt(1);
    time_interval = millis() - rpm_update_before
    rpm_arr[0] = (60000.0 * rpmcount[0]) / ( hall_num[0] * time_interval );
    rpm_arr[1] = (60000.0 * rpmcount[1]) / ( hall_num[1] * time_interval );
    rpmcount[0] = 0;
    rpmcount[1] = 0;
    rpm_update_before = millis();
    attachInterrupt(i, rpm_fun[0], FALLING); // 인터럽트 0->2번핀 1->3번핀
    attachInterrupt(i, rpm_fun[1], FALLING); // 인터럽트 0->2번핀 1->3번핀
  }
}
