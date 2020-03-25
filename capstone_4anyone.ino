#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

volatile int rpmcount[2] = {0, 0};
const float circumference = 2.2; //바퀴둘레 : 2.2m
const int hall_num[2] = {12, 8}; //홀센서 갯수
const int rpm_update_interval = 1000; //1000ms마다 업데이트
boolean recently_stopped = false; //달리다가 최근에 멈췄어?
int speed; //setup에서 초기화 예정, 단수

void rpm_fun1() {// 인터럽트로 카운트 증가하는 함수
  rpmcount[0]++;
}
void rpm_fun2() {
  rpmcount[1]++;
}

float* rpm_calc();
void lcd_init();
void lcd_update(float rpm_arr[]);
void rpm_check(float rpm_arr[]);
void button_check();

void setup() {
  Serial.begin(115200);
  lcd_init();
  attachInterrupt(0, rpm_fun1(), FALLING); // 인터럽트 0->2번핀 1->3번핀
  attachInterrupt(1, rpm_fun2(), FALLING); // 인터럽트 0->2번핀 1->3번핀
  speed = EEPROM.read(64)
}

void loop() {
    float * rpm_arr = rpm_calc();//실패하면 -1,-1로 구성된 값, 성공하면 최소 0이상
    if (rpm_arr[0] >= 0){
        rpm_check(rpm_arr);
        lcd_update(rpm_arr);
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

void lcd_update(float rpm_arr[]) {
    lcd.setCursor(5, 1);   // 3,1에서 글쓰기 시작
    lcd.print("Cadence");
    Serial.print("rpm : ");
    lcd.setCursor(5, 2);
    lcd.print("   "); // LCD에 새로운 rpm값을 계속 표시해주기 위해서 이전에 표시되었던 내용을 지워주는 과정
    lcd.setCursor(5, 2);   // 5,2에서 글쓰기 시작
    lcd.print((int)rpm_arr[0]);
    Serial.println(rpm_arr[0]);
    lcd.setCursor(9, 2);
    lcd.print("rpm");      // lcd 모니터에 rpm 표시해주기
    Serial.print("speed : ");
    lcd.setCursor(5, 3);
    lcd.print("    ");    //LCD에 새로운 rpm값을 계속 표시해주기 위해서 이전에 표시되었던 내용을 지워주는 과정
    lcd.setCursor(5, 3);   // 5,3에서 글쓰기 시작
    float bike_velocity = ((rpm_arr[1] * circumference * 60 ) /1000);
    lcd.print((int)bike_velocity);
    Serial.println(bike_velocity);
    lcd.setCursor(9, 3);
    lcd.print("km/h");
}

float* rpm_calc() {
    static unsigned long rpm_update_before = 0;
    static float rpm_arr[2] = {0.0, 0.0};
    if ( (millis() - rpm_update_before) >= rpm_update_interval ) { //시간이 지났으면 rpm 계산하기
        detachInterrupt(0);
        detachInterrupt(1);
        time_interval = millis() - rpm_update_before
        rpm_arr[0] = (60000.0 * rpmcount[0]) / ( hall_num[0] * time_interval );
        rpm_arr[1] = (60000.0 * rpmcount[1]) / ( hall_num[1] * time_interval );
        rpmcount[0] = 0;
        rpmcount[1] = 0;
        rpm_update_before = millis();
        attachInterrupt(0, rpm_fun1(), FALLING); // 인터럽트 0->2번핀 1->3번핀
        attachInterrupt(1, rpm_fun2(), FALLING); // 인터럽트 0->2번핀 1->3번핀
        return rpm_arr;
    }
    else { //시간이 아직 모자르면 쓰레기값 보내기
        float temp[2] ={-1 ,-1}
        return temp;
    }
}

void rpm_check(float rpm_arr[]){
    if (rpm_arr[0] == 0.0){ //페달링 않는 중이라면
        if (rpm_arr[1] == 0.0){ //거기에 정지까지 한 상태라면
            if (!recently_stopped){
                speed -= 1;
                EEPROM.write(64, speed);
                recently_stopped = true;
            }

        }
    }
    else{
        recently_stopped = false; //달리고있다면 다음 정지를 위해 false로
    }

}
void button_check(){

}