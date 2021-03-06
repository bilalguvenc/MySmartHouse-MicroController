#ifndef MYDEVICES_H
#define MYDEVICES_H

#include <Servo.h>
#include "types.h"
#include <map>
#include <DHT.h>

#define DOOR_OPEN 90
#define DOOR_CLOSE 0


namespace MyDevices
{
  DHT dht(PS_Sicaklik, DHT11);
  Servo Servo_O_Pencere, Servo_Y_Pencere;
  int O_Klima_Derece;

  std::map<device, bool> device_states =
  {
    {device::O_Lamba  , false},
    {device::O_Pencere, false},
    {device::O_Klima  , false},

    {device::Y_Lamba  , false},
    {device::Y_Pencere, false},

    {device::M_Lamba  , false},
    {device::M_Kettle , false},

    {device::A_Hirsiz , false},
    {device::A_Yangin , false},
  };

  int GetServoCommand(bool state)
  {
    return state ? DOOR_OPEN : DOOR_CLOSE;
  }

  float GetTemperature()
  {
    float temp = dht.readTemperature();
    
    if (isnan(temp)) 
    {
      Serial.println(" HATA: Sicaklik Sensoru Okunamadi!");
      return -1.0;
    }

    return temp;
  }

  bool GetMotion()
  {
    int motion = digitalRead(device_pin::PS_Hareket);
    return motion == HIGH;
  }

  void UpdateAC()
  {
    if (MyDevices::device_states[device::O_Klima] == OFF)
    {
      digitalWrite(device_pin::PO_Klima_C, OFF);
      digitalWrite(device_pin::PO_Klima_H, OFF);
      return;
    }

    float temp = MyDevices::GetTemperature();

    if (temp > O_Klima_Derece+1)
    {
      digitalWrite(device_pin::PO_Klima_H, OFF);
      digitalWrite(device_pin::PO_Klima_C, ON);
      return;
    }

    if (O_Klima_Derece-1 > temp)
    {
      digitalWrite(device_pin::PO_Klima_H, ON);
      digitalWrite(device_pin::PO_Klima_C, OFF);
      return;
    }

    digitalWrite(device_pin::PO_Klima_H, ON);
    digitalWrite(device_pin::PO_Klima_C, ON);
    return;
  }

  bool GetAlarm(device dev)
  {
    if (MyDevices::device_states[dev] == OFF)
      return false;

    switch (dev)
    {
      case device::A_Hirsiz:
        return (MyDevices::GetMotion());

      case device::A_Yangin:
        return (MyDevices::GetTemperature() > 22.0F);

      default:
        return false;
    }
    return false;
  }
  
  bool SetDeviceState(device dev, bool state, float degree)
  {
    if (!isDeviceValid(dev))
    {
      Serial.print("HATA: Cihaz bulunamadi! (");
      Serial.print(dev);
      Serial.println(")");
      return false;
    }

    switch (dev)
    {
      case device::O_Lamba:
      case device::Y_Lamba:
      case device::M_Lamba:
      case device::M_Kettle:
        digitalWrite(getDevicePin(dev), state);
        break;

      case device::O_Pencere:
        Servo_O_Pencere.write(GetServoCommand(state));
        break;

      case device::Y_Pencere:
        Servo_Y_Pencere.write(GetServoCommand(state));
        break;

      case device::O_Klima:
        if (state) MyDevices::O_Klima_Derece = degree;
        MyDevices::UpdateAC();
        break;
      case device::A_Hirsiz:
        if (state) Serial.println("Hirsiz alarmi aktif edildi.");
        else Serial.println("Hirsiz alarmi kapatildi.");
        break;
      case device::A_Yangin:
        if (state) Serial.println("Yangin alarmi aktif edildi.");
        else Serial.println("Yangin alarmi kapatildi.");
        break;

      default:
        return false;
    }

    MyDevices::device_states[dev] = state;

    return true;
  }

  bool SetDeviceState(device dev, bool state)
  {
    if (!isDeviceValid(dev))
    {
      Serial.print("HATA: Gecersiz cihaz! (");
      Serial.print(dev);
      Serial.println(")");
      return false;
    }

    if (dev == device::O_Klima && state == ON)
    {
      Serial.println("HATA: Klima derecesi ayarlanmadi!");
      return false;
    }

    return SetDeviceState(dev, state, 0.0F);
  }
  
  void init()
  {
    pinMode(device_pin::PO_Lamba, OUTPUT);
    pinMode(device_pin::PO_Klima_H, OUTPUT);
    pinMode(device_pin::PO_Klima_C, OUTPUT);
    Servo_O_Pencere.attach(device_pin::PO_Pencere);

    // pinMode(device_pin::PY_Lamba, OUTPUT);
    Servo_Y_Pencere.attach(device_pin::PY_Pencere);
    
    pinMode(device_pin::PM_Lamba, OUTPUT);
    pinMode(device_pin::PM_Kettle, OUTPUT);
    
    pinMode(device_pin::PS_Hareket, INPUT);
    dht.begin();
    
    SetDeviceState(device::O_Lamba, OFF);
    SetDeviceState(device::O_Pencere, OFF);
    SetDeviceState(device::O_Klima, OFF);
    UpdateAC();

    SetDeviceState(device::Y_Lamba, OFF);
    SetDeviceState(device::Y_Pencere, OFF);

    SetDeviceState(device::M_Lamba, OFF);
    SetDeviceState(device::M_Kettle, OFF);

    SetDeviceState(device::A_Hirsiz, OFF);
    SetDeviceState(device::A_Yangin, OFF);
  }
}

#endif