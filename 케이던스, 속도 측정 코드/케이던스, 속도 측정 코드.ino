//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display





 
volatile int rpmcount;
volatile int rpmcount2;
int sensorState = 0;
unsigned int rpm ;
unsigned long timeold;
unsigned int rpm2 ;
unsigned long timeold2;
//unsigned long timehold;
const long circumference = 220;
unsigned long bikespeed;
//float bikespeed;


void rpm_fun()
{
  rpmcount++;
  }

void rpm_fun2()
{
  rpmcount2++;
}

void setup() {

  lcd.init();                      // initialize the lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();

    lcd.setCursor(3, 0);   // 3,0���� �۾��� ����
    lcd.print("Cadence meter");

    lcd.setCursor(3, 1);   // 3,1���� �۾��� ����
    lcd.print("initialized!");

    delay(800);

    lcd.clear();

    delay(500);

    lcd.setCursor(5, 1);   // 3,1���� �۾��� ����
    lcd.print("Cadence");                           // ���̴��� ���� �ʱ�ȭ! �޽��� �߰����� ���̴��� �ܾ� ����Ʈ

  Serial.begin(9600);
  attachInterrupt(0, rpm_fun, FALLING); // 2�� �� : 0 , 3�� �� : 1 /// 2�� �ɿ� interrupt�� �ְڴ�.
  attachInterrupt(1, rpm_fun2, FALLING); // ������ �ɿ� �ش��ϴ� Ȧ������ �ڼ��� Ž���Ǿ high->low �ɶ����� ���ͷ�Ʈ�Ǽ� rpm_fun, rpm_fun2 �Լ� ���� -> Ž��Ƚ�� ī��Ʈ

  rpmcount = 0;
  rpmcount2 = 0;
  rpm = 0;
  rpm2 = 0;
  timeold = 0;
  timeold2 = 0;
  //timehold = 0;
}

void loop() { 

    delay(1000); // 1�ʸ��� ��޸��� �޹��� rpm�� �����ؼ� ����Ϳ� ǥ�����ֱ� ����. ��޸� rpm�� ���� �˰������� key, �޹��� rpm�� �������� �ӵ��� �����ϱ� ����
                 // ��޸� rpm �� ���� �̿��ؼ� �ӵ��� ����Ҽ��� ������, ��޸��� ���� �ʴ� ����� ���� �����Ͽ� �޹��� rpm�� ���� �����ϱ�� �Ͽ���.
                 // ������ �� �����̸� �������� ���� �������͸� �����ϴ� �ڵ�� ��ġ�� �������� ���������� �۵��� ���ϰ� �� �� ���߿� �ڵ� ��ü������ ��ġ���� �̺κ��� ���ľ��ҵ�.



    //��޸� rpm ����//             



    detachInterrupt(0); // rpm�� �����ϴ� ���ȿ��� 2������ ���ͷ�Ʈ ��� �ߴ�.
    
    lcd.setCursor(5, 2);
    lcd.print("   "); // LCD�� ���ο� rpm���� ��� ǥ�����ֱ� ���ؼ� ������ ǥ�õǾ��� ������ �����ִ� ���� 
    
    rpm = (60000 * rpmcount) / (12*(millis() - timeold)); //the 12 changes to 4 if 4 magnets are used. , ��޸� rpm �� ���ϴ� ����. ��޸� �ʿ� �ڼ� 12���� �޷��־�
                                                          //�ѹ��� �������� �ڼ� 12���� ����. �־����ð�(millis()-timeold)���� Ž���� �ڼ��Ǽ� = rpmcount , ���� 12�� ������ �ָ� 
                                                          //�����ð���(���⼱ ms) �����Ű� ����� ������ ���� ������, �츮�� ���ϴ°� round per minute�̴ϱ� ������ ���߱� ���� 
                                                          // 60000 �� ���Ͽ���. 
    timeold = millis(); // ���ؽð� ����
    rpmcount = 0;
    
    Serial.println(timeold);
    Serial.println(millis());

    //if( (millis() - timehold ) > 1000 ) { <- �տ� �ִ� delay(1000)�� ���ְ� �; �̷������� �ڵ��ؾ߰ڴٴ� ������ ������ �׶� ��� �ð��� �ɷº�������.. ����
     
    
    lcd.clear();
    lcd.setCursor(5, 1);   // 3,1���� �۾��� ����
    lcd.print("Cadence");
    lcd.setCursor(5, 2);   // 5,2���� �۾��� ����
    lcd.print(rpm);
    lcd.setCursor(9, 2);
    lcd.print("rpm");      // lcd ����Ϳ� rpm ǥ�����ֱ�~    , ������Ÿ���̶� ���������� �����ֱ������ ������� ���������� �ڵ��� ���ø����̼��� ���� �ڵ��� ȭ���� ���ؼ� �����ֵ��� 
                           // �Ϸ�����

    //timehold = millis();
   

    attachInterrupt(0,rpm_fun,FALLING); // rpm ���� �������Ƿ� �ٷ� �ٽ� interrupt �ֱ�




    //�ӵ�����//

    // �������� ��������, ������ �̰� �޹���(�ӵ�����)�� //

    detachInterrupt(1); 
    
    lcd.setCursor(5, 3);
    lcd.print("   ");    //LCD�� ���ο� rpm���� ��� ǥ�����ֱ� ���ؼ� ������ ǥ�õǾ��� ������ �����ִ� ���� 

    rpm2 = (60000 * rpmcount2) / (8*(millis() - timeold2));  // ������ �����ѰͰ� ������ �޹������� �ڼ��� 8�� �����.
    bikespeed = ((rpm2 * circumference * 60 ) / 100000); // �޹��� rpm�� �޹��� ����(220cm)�� �̿��ؼ� �ӵ��� ��ȯ�ϴ� ����.
    timeold2 = millis(); // ���ؽð� ����
    rpmcount2 = 0;
    
    
    
    lcd.setCursor(5, 3);   // 5,2���� �۾��� ����
    lcd.print(bikespeed);
    lcd.setCursor(9, 3);
    lcd.print("km/h");
    
    attachInterrupt(1,rpm_fun2,FALLING);

}