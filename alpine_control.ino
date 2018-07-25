/* Скетч Arduino для управления ГУ Alpine через проводное соединение mini-jack */
/* Изначально заточено под кнопки на руле Citroen XM */
/* Роман Ядровский */
/* http://cx.podolsk.ru/board/memberlist.php?mode=viewprofile&u=3887 */
/* http://www.drive2.ru/r/citroen/288230376152072501 */

// Команды для ГУ Alpine
#define cmdPower      0x09
#define cmdSource     0x0A
#define cmdBand       0x0D
#define cmdVolumeDown 0x15
#define cmdVolumeUp   0x14
#define cmdUp         0x0E
#define cmdDown       0x0F
#define cmdLeft       0x13
#define cmdRight      0x12
#define cmdPlay       0x07
#define cmdMute       0x16
#define cmdCDChange   0x03

// Маски кнопок (нужно для сочетаний нажатий)
#define MaskButtonEXM    1
#define MaskButtonSEEK   2
#define MaskButtonVOL_P  4
#define MaskButtonVOL_M  8

#define ControlCommon      3 // Масса кнопок на пине D3
#define ControlButtonEXM   4 // Кнопка EXM на пине D4
#define ControlButtonSEEK  5 // Кнопка SEEK на пине D5
#define ControlButtonVOL_P 6 // Кнопка VOLUME+ на пине D6
#define ControlButtonVOL_M 7 // Кнопка VOLUME- на пине D7

#define Alpine 2 // Посылать управляющие сигналы на D2

int longPressDuration = 2000;
int tempTimer;
byte lastCode;
boolean longPressed;

void setup() 
{
  Serial.begin(115200); 

  pinMode(ControlButtonEXM, INPUT);
  digitalWrite(ControlButtonEXM, HIGH);

  pinMode(ControlButtonSEEK, INPUT);
  digitalWrite(ControlButtonSEEK, HIGH);

  pinMode(ControlButtonVOL_P, INPUT);
  digitalWrite(ControlButtonVOL_P, HIGH);

  pinMode(ControlButtonVOL_M, INPUT);
  digitalWrite(ControlButtonVOL_M, HIGH);

  pinMode(ControlCommon, OUTPUT);

  pinMode(Alpine, OUTPUT);
}

void loop(void) 
{
  byte code = scan();
  
  // Магия долгого нажатия
  if(code)
  {
    if(longPressed)
    {
      return;
    }
    if(code != lastCode)
    {
      lastCode = code;
      tempTimer = millis();
    } else if (code == lastCode) {
      int currentDuration = millis() - tempTimer;
      if(currentDuration > longPressDuration)
      {
        longPressed = true;
        delay(100); // Тактическая пауза, иначе ГУ не поймет
      }
    }
  } else {
    lastCode = 0;
    longPressed = false;
  }
  
  // Здесь программируем кнопки
  
  if(code == (MaskButtonSEEK | MaskButtonEXM))
  {
    SendCommand(cmdSource);
  }
  
  if(code == (MaskButtonSEEK | MaskButtonVOL_P))
  {
    SendCommand(cmdRight);
  }
  
  if(code == (MaskButtonSEEK | MaskButtonVOL_M))
  {
    SendCommand(cmdLeft);
  }
  
  if (code == MaskButtonEXM)
  {
    if(longPressed)
    {
      SendCommand(cmdPower);
    }
    else {
      SendCommand(cmdMute);
    }
  }
  
  if(code == MaskButtonVOL_P)
  {
    SendCommand(cmdVolumeUp);
  }
  
  if(code == MaskButtonVOL_M)
  {
    SendCommand(cmdVolumeDown);
  }
}

// Определяем нажатую кнопку
byte scan(void)
{
  byte code = 0;
  digitalWrite(ControlCommon, LOW);
  if(digitalRead(ControlButtonEXM) == LOW)
  {
    code |= MaskButtonEXM;
  } 
  if(digitalRead(ControlButtonSEEK) == LOW)
  {
    code |= MaskButtonSEEK;
  } 
  if(digitalRead(ControlButtonVOL_P) == LOW)
  {
    code |= MaskButtonVOL_P;
  } 
  if(digitalRead(ControlButtonVOL_M) == LOW)
  {
    code |= MaskButtonVOL_M;
  } 

  digitalWrite(ControlCommon, HIGH);

  return code;
}

// Отправка байта (протокол NEC-Clarion)
void SendByte(byte data)
{
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(Alpine, HIGH);
    delayMicroseconds(560); // Пауза между битами
    digitalWrite(Alpine, LOW);

    if (data & 1) // Последний бит умножаем на 1
    {
      delayMicroseconds(1680); // Логическая единица
    }
    else
    {
      delayMicroseconds(560); // Логический ноль
    }

    data >>= 1; // Новый, сдвинутый вправо байт (на 1 бит)
  }
}

// Отправка команды
void SendCommand(byte command)
{
  // Что-то вроде приветствия,
  // особенность модифицированного протокола NEC
  digitalWrite(Alpine, HIGH);
  delayMicroseconds(9000);
  digitalWrite(Alpine, LOW);
  delayMicroseconds(4500);

  // Адрес протокола (общедоступная справочная инфа)
  SendByte(0x86);
  SendByte(0x72);
  
  // Отправка команды
  SendByte(command);
  SendByte(~command);

  // Для определения последнего бита (конец посылки)
  digitalWrite(Alpine, HIGH); 
  delayMicroseconds(560); 
  digitalWrite(Alpine, LOW);
  
  delay(50); // Техническая пауза
}
