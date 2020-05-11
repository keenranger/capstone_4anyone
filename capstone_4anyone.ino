#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Servo.h>
//핀
int button_pin[3] = {11, 12, 13}; //위, 가운데, 아래
int motor_pin[2] = {9, 10}; //앞서보, 뒷서보
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
Servo front_servo;
Servo rear_servo;
//RPM, 속도 계산 관련
volatile int rpmcount[2] = {0, 0};
const float circumference = 2.2; //바퀴둘레 : 2.2m
const int hall_num[2] = {12, 8}; //홀센서 갯수
const int rpm_update_interval = 1000; //1000ms마다 업데이트
//변속 관련
int speed; //setup에서 초기화 예정, 단수
const int minimum_rpm = 10; //10rpm 이하면 달리다가 발가만히두는거라고생각
const int speed_lower_threshold[5] = {50, 55, 55, 60, 65};
const int speed_heighten_threshold[5] = {70, 70, 75, 80, 80};
boolean recently_stopped = true; //달리다가 최근에 멈췄어? 켰을때 멈춘상태로 시작이므로 true로
boolean recently_lower = false; //최근에 단이 내려갔는가 (모터 각도 때문)
boolean recently_heighten = false; //최근에 단이 올랐는가 (모터 각도 때문)
//버튼 관련
boolean last_button_state[3] = {false, false, false};
boolean button_state[3] = {false, false, false};
boolean button_queue[3] = {false, false, false};
unsigned long last_debounce[3] = {0, 0, 0};
const int debounce_delay = 50;
//모드 관련
boolean auto_mode = true; //기본으론 auto 모드로
//모터 관련
int last_speed;
int speed_gear[10][2] = {{0, 6}, {0, 5}, {0, 4}, {0, 3}, {1, 3}, {1, 2}, {1, 1}, {2, 2}, {2, 1}, {2, 0}};
int front_degree[3] = {100, 72, 58};
int front_lower_degree[2] = {100, 76};
int front_heighten_degree[2] = {68, 54};
int rear_degree[7] = {57, 65, 75, 85, 98, 121, 145};
int rear_lower_degree[6] = {57, 65, 73, 85, 98, 121};
int rear_heighten_degree[6] = {65, 77, 85, 98, 121, 145};
const int gear_select_interval = 200; //200ms마다 기어선택

void rpm_fun1() {// 인터럽트로 카운트 증가하는 함수
  rpmcount[0]++;
}
void rpm_fun2() {
  rpmcount[1]++;
}

float* rpm_calc();
void lcd_init();
void lcd_update(float rpm_arr[]);
void stop_check(float rpm_arr[]);
void rpm_check(float rpm_arr[]);
void button_check(int);
void queue_processor(int);
void gear_selector();

void setup() {
  Serial.begin(115200);
  lcd_init();
  attachInterrupt(0, rpm_fun1, FALLING); // 인터럽트 0->2번핀 1->3번핀
  attachInterrupt(1, rpm_fun2, FALLING); // 인터럽트 0->2번핀 1->3번핀
  for (int i = 1; i < 3; i++) { //버튼 핀 4개 인풋 활성화
    pinMode(button_pin[i], INPUT);
  }
  front_servo.attach(motor_pin[0]);
  rear_servo.attach(motor_pin[1]);
  speed = EEPROM.read(64); //정차 전 단수 불러오기
  if ( (speed < 1) || (speed > 10) ) { //EEPROM이 초기화가 안됐다면 = 만약 speed(1~10)가 초기화안된상태라면
    speed = 3;
  }
  last_speed = speed; // 읽어온 단수
}

void loop() {
  for (int i = 0; i < 3; i++) {//버튼 눌렸는지 체크하는 부분
    button_check(i);
    queue_processor(i);
  }
  gear_selector();

  float * rpm_arr = rpm_calc();//실패하면 -1,-1로 구성된 값, 성공하면 최소 0이상
  if (rpm_arr[0] >= 0) {
    stop_check(rpm_arr);
    lcd_update(rpm_arr);
    Serial.print("SPD : ");
    Serial.println(speed);
    if (auto_mode == true) { //자동 모드일 때만 자동 변속
      rpm_check(rpm_arr);
    }
  }
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

float* rpm_calc() {
  static unsigned long rpm_update_before = 0;
  static float rpm_arr[2] = {0.0, 0.0};
  if ( (millis() - rpm_update_before) >= rpm_update_interval ) { //시간이 지났으면 rpm 계산하기
    detachInterrupt(0);
    detachInterrupt(1);
    unsigned long time_interval = millis() - rpm_update_before;
    rpm_arr[0] = (60000.0 * rpmcount[0]) / ( hall_num[0] * time_interval );//ms -> min = 60000곱함
    rpm_arr[1] = (60000.0 * rpmcount[1]) / ( hall_num[1] * time_interval );
    rpmcount[0] = 0;
    rpmcount[1] = 0;
    rpm_update_before = millis();
    attachInterrupt(0, rpm_fun1, FALLING); // 인터럽트 0->2번핀 1->3번핀
    attachInterrupt(1, rpm_fun2, FALLING); // 인터럽트 0->2번핀 1->3번핀
    return rpm_arr;
  }
  else { //시간이 아직 모자르면 쓰레기값 보내기
    rpm_arr[0] = -1;
    return rpm_arr;
  }
}

void stop_check(float rpm_arr[]) {
  if ( (rpm_arr[0] == 0.0) && (rpm_arr[1] == 0.0) ) { //페달링 않고 정지한다면
    if (!recently_stopped) { //최근에 멈추지 않았다면
      if ( speed > 3) { //1단에서 더낮출수없음 : 3초과면 낮춤
        speed -= 1;
      }
      EEPROM.write(64, speed);
      recently_stopped = true;
    }
  }
  else {
    recently_stopped = false; //달리고있다면 다음 정지를 위해 false로
  }

}
void rpm_check(float rpm_arr[]) {
  if (rpm_arr[0] > 10) { //페달링을 하고있는 상태에서만 자동으로 단을 바꿈
    if ( (speed >= 3) && (speed <= 7) ) { //1단 ~ 5단에서만 단 상승 검사
      if (rpm_arr[0] > speed_heighten_threshold[speed - 3]) { //rpm이 기준보다 높으면,
        speed += 1;
      }
    }
    if ( (speed >= 4) && (speed <= 8) ) { //2단 ~ 6단에서만 단 하강 검사
      if (rpm_arr[0] < speed_lower_threshold[speed - 4]) { //rpm이 기준보다 낮으면,
        speed -= 1;
      }
    }

  }
}
void button_check(int i) {
  int reading = digitalRead(button_pin[i]);
  if (reading != last_button_state[i])  //스위치의 이전과 지금 상태가 다르면
    last_debounce[i] = millis();   //초를 기록합니다.

  if ((millis() - last_debounce[i]) > debounce_delay) { //스위치 상태가 debounce_delay 이상 같으면
    if (reading != button_state[i]) { //state와 상태가 다를경우 기록
      button_state[i] = reading;
      if (button_state[i] == false) { //LOW일때 작업 할당 -> pull-up 저항있음
        button_queue[i] = true;
        Serial.print(i+1);
        Serial.println(" button pressed!");
      }
    }
  }
  last_button_state[i] = reading;
}

void queue_processor(int i) {
  if (button_queue[i] == true) {
    switch (i) {
      case 0://위로버튼
        auto_mode = false;
        if (speed < 10){
          speed += 1;
        }
        button_queue[i] = false;
        break;
      case 1:
        if (auto_mode == false) { //수동 -> 자동일때는
          if (speed <= 2) { //L1, L2일땐 A1으로
            speed = 3;
          }
          else if (speed >= 9) { //H1, H2일땐 A6으로
            speed = 8;
          }
          auto_mode = true;
        }
        else { //자동 -> 수동일때는
          auto_mode = false;
        }
        button_queue[i] = false;
        break;
      case 2:
        auto_mode = false;
        if (speed > 1){
          speed -= 1;
        }
        button_queue[i] = false;
        break;
    }

  }

}

void gear_selector(){
  static unsigned long gear_select_before = 0;
  if ( (millis() - gear_select_before) >= gear_select_interval ) { 
//    speed_gear[speed][0]
    if (speed_gear[speed - 1][0] > speed_gear[last_speed - 1][0]){//앞바퀴 기어가 올라가는 거라면
      front_servo.write(front_heighten_degree[speed_gear[speed - 1][0] - 1]);
    }
    else if (speed_gear[speed - 1][0] < speed_gear[last_speed -1][0]){//앞바퀴 기어가 내려가는 거라면
      front_servo.write(front_lower_degree[speed_gear[speed - 1][0]]);
    }
    else{//앞기어가 변하지않았다면
      front_servo.write(front_degree[speed_gear[speed - 1][0]]);
    }
    if (speed_gear[speed - 1][1] > speed_gear[last_speed - 1][1]){//뒷바퀴 기어가 올라가는 거라면
      rear_servo.write(rear_heighten_degree[speed_gear[speed - 1][1] -1]);
    }
    else if (speed_gear[speed - 1][1] < speed_gear[last_speed - 1][1]){//뒷바퀴 기어가 내려가는 거라면
      rear_servo.write(rear_lower_degree[speed_gear[speed - 1][1]]);
    }
    else{//뒷기어가 변하지않았다면
      rear_servo.write(rear_degree[speed_gear[speed - 1][1]]);
    }
    last_speed = speed; //마지막으로 lastspeed 에 현재 speed 넣어줌
    gear_select_before = millis();
  }
}

void lcd_update(float rpm_arr[]) {
  lcd.clear();
  lcd.setCursor(1, 1);   // 3,1에서 글쓰기 시작
  lcd.print("Cadence");
  lcd.setCursor(11, 1);   // 5,2에서 글쓰기 시작
  lcd.print((int)rpm_arr[0]);
  lcd.setCursor(15, 1);
  lcd.print("rpm");      // lcd 모니터에 rpm 표시해주기
  lcd.setCursor(1, 2);   // 5,3에서 글쓰기 시작
  lcd.print("Velocity");
  float bike_velocity = ((rpm_arr[1] * circumference * 60 ) / 1000); //60초동안, km니까 나누기천
  lcd.setCursor(11, 2);
  lcd.print((int)bike_velocity);
  lcd.setCursor(15, 2);
  lcd.print("km/h");
  lcd.setCursor(1, 3);
  lcd.print("SPD");
  lcd.setCursor(11, 3);
  if (speed >= 9){//h1,h2일땐
    lcd.print("H");
    lcd.setCursor(11, 4);
    lcd.print(speed - 8);
  }
  else if (speed <= 2){//L1,L2일땐
    lcd.print("L");
    lcd.setCursor(11, 4);
    lcd.print(speed);
  }
  else{// 1~6단일땐
    if (auto_mode == true){//자동모드일때 a표시
      lcd.print("A");
    }
    else{
      lcd.print("M");
    }
    lcd.setCursor(11, 4);
    lcd.print(speed - 2);
  }
  
}