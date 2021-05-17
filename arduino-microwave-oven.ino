#include <TM1637.h>   //Grove segment display library
#include <Encoder.h>  //Paul Stoffregen's encoder library
#include <TimerOne.h>
#include <EEPROM.h>

#define BUZZER_pin         11 /* Active buzzer pin. */
#define DISPLAY_DATA_pin    8 /* TM1637 display data pin. */
#define DISPLAY_CLK_pin     7 /* TM1637 display clock pin. */
#define ENCODER_SW_pin      6 /* Rotary encoder button pin. */
#define ENCODER_DT_pin      5 /* Rotary encoder DT pin. */
#define ENCODER_CLK_pin     4 /* Rotary encoder CLK pin. */
#define MICROWAVE_RELAY_pin 2 /* Relay pin, that turns microwave power on. */
#define MOTOR_FAN_RELAY_pin 9 /* Relay pin, that turns cooler fan and food rotation motor on. */

#define POWER_max        5 /* Power levels number. */
#define POWER_CNTR_mul   4 /* Seconds per one power level. POWER_max * POWER_CNTR_mul = total power cycle time. */
#define SECONDS_max      1200 /* Maximal working time. */
#define SECONDS_CNTR_mul 3 /* Seconds per encoder rotation tik sensivity. */
#define SECONDS_CNTR_max SECONDS_max * SECONDS_CNTR_mul
#define TIME_CNTR_SPEEDUP 30 /* Timer value, after that we should increase the time faster by encoder rotation. */
#define FAN_AFTER_OFF_TIME 5 /* Time to keep fan on after microwaving finished. */

TM1637 disp(DISPLAY_CLK_pin,DISPLAY_DATA_pin);
Encoder knob(ENCODER_DT_pin, ENCODER_CLK_pin);

static int time_counter = 0;
static uint32_t power = 1;
static uint32_t power_counter = 0;
static uint32_t relay_state = 0;
static uint32_t motor_fan_state = 0;
static uint32_t motor_fan_counter = 0;
static long oldPosition = 0;
unsigned long oldMills;
static int finish = 0;
static int updateDisplay = 0;
static int updatePower = 0;
static int displayPower = 0;
static int silent_off = 0;
static int reduse_noise = 0;

static int melody_durations[] = {
6, 8, 8, 8, 16, 10, 10
};

static int pause_durations[] = {
16, 4, 4, 8, 18, 6, 6
};

/* Melody play at the end, used an active buzzer that sounds on power on by itself. */
void beep(){
  
  for (int i = 0; i < sizeof(melody_durations)/sizeof(melody_durations[0]); i++)
  {
    digitalWrite(BUZZER_pin, HIGH);
    delay((melody_durations[i]/2) * 30);
    digitalWrite(BUZZER_pin, LOW);
    delay((pause_durations[i]/2) * 20);
  }
}

/* Turn the microwave on/off */
void set_microwave_relay(uint8_t state){
  /* Turn the fan and the plate rotation on before turn microwave on. */
  if (state)
  {
    set_motor_fan(1);
  }
  
  relay_state = state?1:0;
  digitalWrite(MICROWAVE_RELAY_pin, state?0:1);
}

/* Turn the fan and the plate rotation on/off. */
void set_motor_fan(uint8_t state){
  /* Enable noise reducing. */
  if (!state)
  {
    reduse_noise = 2;
  }

  motor_fan_state = state?1:0;
  digitalWrite(MOTOR_FAN_RELAY_pin, state?0:1);
  motor_fan_counter = 0;
}

/* 1/second interrupt callback. */
void timerCallback()
{
  /* IDLE state. */
  if (!time_counter && !relay_state && !finish)
  {
    /* Keep fan and rotation motors several more seconds on after microwave off. */
    if (motor_fan_state)
    {
      motor_fan_counter++;
  
      if (motor_fan_counter > FAN_AFTER_OFF_TIME)
      {
        set_motor_fan(0);
      }
    }

    /* Encoder press process, show power level. On long press- increase the level. */
    if (!digitalRead(ENCODER_SW_pin))
    {
      if (!displayPower)
      {
        displayPower = 1;
      } else
      {
        updatePower = 1;
      }
    }
  }

  /* Working state, process time counting and the power regulation. */
  if (!finish && (time_counter || relay_state))
  {
    time_counter -= SECONDS_CNTR_mul;

    /* Time ended. */
    if (time_counter <= 0)
    {
      time_counter = 0;
      finish = 1;
      power_counter = 0;
      set_microwave_relay(0);
    } else
    {
      /* Regulate power depending on power level settings. */
      if (power_counter < (power * POWER_CNTR_mul))
      {
        set_microwave_relay(1);
      } else
      {
        set_microwave_relay(0);
      }
      
      power_counter++;

      /* Loop the power counter. */
      if (power_counter >= (POWER_max * POWER_CNTR_mul))
      {
        power_counter = 0;
      }
    }
    updateDisplay = 1;
  }

  /* Reduse noise influence on encoder. */
  if (reduse_noise)
  {
    reduse_noise--;
  }
}

void setup() {
  pinMode(MICROWAVE_RELAY_pin, OUTPUT);
  pinMode(MOTOR_FAN_RELAY_pin, OUTPUT);
  pinMode(BUZZER_pin, OUTPUT);
  
  pinMode(DISPLAY_DATA_pin, OUTPUT);
  pinMode(DISPLAY_CLK_pin, OUTPUT);
  
  pinMode(ENCODER_SW_pin, INPUT_PULLUP);
  pinMode(ENCODER_DT_pin, INPUT);
  pinMode(ENCODER_CLK_pin, INPUT);

  set_microwave_relay(0);
  set_motor_fan(0);

  /* Restore power settings from EEPROM. */
  power = constrain(EEPROM.read(0),1,POWER_max);

  disp.init();
  disp.set(0);
  
  disp.point(1);
  disp.clearDisplay();

  /* 1/s time init. */
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(timerCallback);

  /* First position of rotary encoder. */
  oldPosition = knob.read();
}

/* Display the timer value. */
void display_seconds(uint32_t seconds) {
  int minutes;

  seconds = (seconds / SECONDS_CNTR_mul);

  if (!seconds)
  {
    disp.point(1);
    disp.clearDisplay();
  } else
  {
    minutes = seconds / 60;
    seconds = seconds % 60;
  
    disp.point(1);
    disp.display(3, seconds % 10);
    disp.display(2, seconds / 10 % 10);
    disp.display(1, minutes % 10);
    disp.display(0, minutes / 10 % 10);
  }
}

void loop() {
  long newPosition;
  unsigned long curMills;

  if (!reduse_noise)
  {
    newPosition = knob.read();
  } else /* Disable encoder for a second */
  {
    newPosition = oldPosition;
    knob.write(oldPosition);
  }

  /* Encoder rotated right. */
  if (newPosition < oldPosition)
  {
    time_counter += oldPosition - newPosition;

    /* Give some more time at the beginning of rotation. */
    if (time_counter < SECONDS_CNTR_mul)
    {
      time_counter = SECONDS_CNTR_mul * 5;
    }

    /* Adjust rotation sensivity. */
    if ((time_counter / SECONDS_CNTR_mul) > TIME_CNTR_SPEEDUP)
    {
      time_counter += oldPosition - newPosition;
    }

    /* Avoid overflow. */
    if (time_counter > SECONDS_CNTR_max)
    {
      time_counter = SECONDS_CNTR_max;
    }

    updateDisplay = 1;
    oldPosition = newPosition;
  }

  /* Encoder rotated left. */
  if (newPosition > oldPosition)
  {
    time_counter -= newPosition - oldPosition;

    if ((time_counter / SECONDS_CNTR_mul) > TIME_CNTR_SPEEDUP)
    {
      time_counter -= newPosition - oldPosition;
    }

    /* Avoid overflow. */
    if (time_counter < 0)
    {
      time_counter = 0;
      silent_off = 1; /* On manual off do not play melody. */
    }
    
    updateDisplay = 1;
    oldPosition = newPosition;
  }

  /* Time finished. */
  if (finish)
  {
    Timer1.stop();
    time_counter = 0;
    display_seconds(time_counter);
    if (!silent_off)
    {
      beep();
    }

    finish = 0;
    silent_off = 0;
    Timer1.start();
  }

  /* Update display if needed. */
  if (updateDisplay)
  {
    display_seconds(time_counter);
    updateDisplay = 0;
  }

  /* Display power level on short press and increase it on the long one. */
  if (displayPower || updatePower)
  {
    if (updatePower)
    {
      power++;
      if (power > POWER_max)
      {
        power = 1;
      }
    
      EEPROM.update(0, power);
      updatePower = 0;
    }
    
    disp.clearDisplay();
    disp.display(0, power);
  }

  /* Encoder button was released, hide the power level info. */
  if (displayPower && digitalRead(ENCODER_SW_pin))
  {
    displayPower = 0;
    disp.clearDisplay();
  }
}
