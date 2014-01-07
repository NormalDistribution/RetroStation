int startButton = PIN_B0;
int selectButton = PIN_B1;
int aButton = PIN_B2;
int bButton = PIN_B3;
int cButton = PIN_B7;
int xButton = PIN_D0;
int yButton = PIN_D1;
int zButton = PIN_D2;

void setup() 
{
  pinMode(startButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(aButton, INPUT_PULLUP);
  pinMode(bButton, INPUT_PULLUP);
  pinMode(cButton, INPUT_PULLUP);
  pinMode(xButton, INPUT_PULLUP);
  pinMode(yButton, INPUT_PULLUP);
  pinMode(zButton, INPUT_PULLUP);
}

void loop() 
{
  if (!digitalRead(startButton))
  {
    Joystick.button(1, true);
  }
  else
  {
    Joystick.button(1, false);
  }
  
  if (!digitalRead(selectButton))
  {
    Joystick.button(2, true);
  }
  else
  {
    Joystick.button(2, false);
  }
  
  if (!digitalRead(aButton))
  {
    Joystick.button(3, true);
  }
  else
  {
    Joystick.button(3, false);
  }
  
  if (!digitalRead(bButton))
  {
    Joystick.button(4, true);
  }
  else
  {
    Joystick.button(4, false);
  }
  
  if (!digitalRead(cButton))
  {
    Joystick.button(5, true);
  }
  else
  {
    Joystick.button(5, false);
  }
  
  if (!digitalRead(xButton))
  {
    Joystick.button(6, true);
  }
  else
  {
    Joystick.button(6, false);
  }
  
  if (!digitalRead(yButton))
  {
    Joystick.button(7, true);
  }
  else
  {
    Joystick.button(7, false);
  }
  
  if (!digitalRead(zButton))
  {
    Joystick.button(8, true);
  }
  else
  {
    Joystick.button(8, false);
  }
}
