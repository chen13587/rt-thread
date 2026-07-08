
#include <Arduino.h>

#define TEST_PIN D13  //使用 D13，对应 J2 Pin 12 (P3_10)

void setup(void)
{
    Serial.begin();
    Serial.println("Hello RTduino!");
    
    //设置引脚为输出模式
    pinMode(TEST_PIN, OUTPUT);
    Serial.println("Pin D13 set to OUTPUT");
}

void loop(void)
{
    //拉高
    digitalWrite(TEST_PIN, HIGH);
    Serial.println("Pin HIGH");
    delay(500);  //高电平持续 500ms
    
    //拉低
    digitalWrite(TEST_PIN, LOW);
    Serial.println("Pin LOW");
    delay(500);  //低电平持续 500ms
}