#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

volatile int rpmcount[2] = {0, 0};
int timeold[2] = {0, 0};
const long circumference = 220;
const int hall_num[2] = {12, 8}; //홀센서 갯수
int lcd_update_before = 0;
int lcd_update_interval = 300; // 300ms마다 업데이트
int rpm_update_before[2] = {0, 0};
int rpm_update_interval = 300; //300ms마다 업데이트
int rpm_arr[2] = {0, 0};

void rpm_fun1() {// 인터럽트로 카운트 증가하는 함수
  rpmcount[0]++;
}
void rpm_fun2() {
  rpmcount[1]++;
}
void (*rpm_fun[2])() = {rpm_fun1, rpm_fun2};    // int형 반환값, int형 매개변수 두 개가 있는 함수 포인터 배열 선언, 첫 번째 요소에 함수의 메모리 주소 저장
void rpm_calc(int i);
void lcd_init();
void lcd_update(int rpm[]);

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

void lcd_update(int rpm[]) {
  if ((millis() - lcd_update_before) >= lcd_update_interval){
    lcd_update_before = millis();
    lcd.setCursor(5, 1);   // 3,1에서 글쓰기 시작
    lcd.print("Cadence");
    Serial.print("rpm : ");
    lcd.setCursor(5, 2);
    lcd.print("   "); // LCD에 새로운 rpm값을 계속 표시해주기 위해서 이전에 표시되었던 내용을 지워주는 과정
    lcd.setCursor(5, 2);   // 5,2에서 글쓰기 시작
    lcd.print(rpm[0]);
    Serial.println(rpm[0]);
    lcd.setCursor(9, 2);
    lcd.print("rpm");      // lcd 모니터에 rpm 표시해주기
    Serial.print("speed : ");
    lcd.setCursor(5, 3);
    lcd.print("   ");    //LCD에 새로운 rpm값을 계속 표시해주기 위해서 이전에 표시되었던 내용을 지워주는 과정
    lcd.setCursor(5, 3);   // 5,3에서 글쓰기 시작
    int bikespeed = ((rpm[1] * circumference * 60 ) / 100000);
    lcd.print(bikespeed);
    Serial.println(bikespeed);
    lcd.setCursor(9, 3);
    lcd.print("km/h");
  }
}

void rpm_calc(int i) {
  if ((millis() = rpm_update_before[i]) >= rpm_update_interval){
    rpm_update_before[i] = millis();
    detachInterrupt(i);
    rpm_arr[i] = int((60000 * rpmcount[i]) / (hall_num[i] * (millis() - timeold[i])));
    timeold[i] = millis(); // 기준시간 리셋
    rpmcount[i] = 0;
    attachInterrupt(i, rpm_fun[i], FALLING); // 인터럽트 0->2번핀 1->3번핀
  }
}