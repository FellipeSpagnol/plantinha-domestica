//Bibliotecas
#include <Arduino.h>
#include <TMP36.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <EEPROM.h>

//Funções
void startScreen();
void mainMenu();
int plantSelection();
void dateDefinition();
void resetMenu();
void plantScreen(int plant_type);

void menuVariableReset();
void EEPROMClear(int exception);
void waterTimeScreen();
void printDigits();

//Variáveis de Controle
bool start = true;
bool date_defined = false;
bool plant_defined = false;
bool choose_reset = false;
bool choose_plant = false;
bool choose_date = false;
int plant_type;

//Outras Variáveis
int arrow_position = 1;
bool auxiliar;

//Definição dos Botões
#define back_button 8
#define down_button 6
#define up_button 4
#define confirm_button 2

//Multitarefa
unsigned long sensor_millis = 0;
unsigned long clock_millis = 0;

int sensor_delay = 1000;
int clock_delay = 1000;

//LCD
#define col 20
#define lin 4
#define ende 0x27
LiquidCrystal_I2C lcd(ende, col, lin);

//Sensores
TMP36 myTMP36(A0, 5.0);

#define ldr_1 A1
#define ldr_2 A2
float value_ldr_1;
float value_ldr_2;
float value_ldr_mean;

float celsius_temperature;

#define hygrometer A3
float humidity;

//Caracteres Especiais
byte right_arrow[8] = {
  B00000,
  B11100,
  B01110,
  B00111,
  B00111,
  B01110,
  B11100,
  B00000
};

byte down_arrow[8] = {
  B00000,
  B00000,
  B10001,
  B11011,
  B11111,
  B11111,
  B01110,
  B00100
};

byte up_arrow[8] = {
  B00100,
  B01110,
  B11111,
  B11111,
  B11011,
  B10001,
  B00000,
  B00000
};

byte full[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

//Situação Da Planta
int water_state = 1;
int celsius_state = 1;
int light_state = 1;
bool plant_healthy;

////Variáveis Adequadas de Cultivo
//Orquídea
float orquidea_high_celsius = 30;
float orquidea_low_celsius = 10;
float orquidea_high_light = 930;
float orquidea_low_light = 350;
float orquidea_water_frequency = 12;  // Intervalo de 12 Horas
//Suculenta
float suculenta_high_celsius = 29;
float suculenta_low_celsius = 10;
float suculenta_high_light = 930;
float suculenta_low_light = 350;
float suculenta_water_frequency = 168;  // Intervalo de 168 Horas
//Bromélia
float bromelia_high_celsius = 35;
float bromelia_low_celsius = 15;
float bromelia_high_light = 1023;
float bromelia_low_light = 350;
float bromelia_water_frequency = 56;  // Intervalo de 56 Horas
//Espada de São Jorge
float espada_high_celsius = 30;
float espada_low_celsius = 10;
float espada_high_light = 1023;
float espada_low_light = 350;
float espada_water_frequency = 168;  // Intervalo de 168 Horas
//Fitônia
float fitonia_high_celsius = 27;
float fitonia_low_celsius = 18;
float fitonia_high_light = 930;
float fitonia_low_light = 300;
float fitonia_water_frequency = 56;  // Intervalo de 56 Horas

//Umidade Adequada
int watered_metter = 550;
int flooding_metter = 250;

//Irrigação
bool auxiliar_water = true;
time_t water_time = 0;

//Determinação Dia e Noite
time_t current_time;
bool day_check;

//Controle EEPROM
int date_defined_address = 1;
int plant_defined_address = 2;
int plant_type_address = 3;
int water_time_address = 4;

void setup() {

  Serial.begin(9600);

  lcd.init();
  lcd.clear();
  lcd.backlight();

  lcd.createChar(0, right_arrow);
  lcd.createChar(1, down_arrow);
  lcd.createChar(2, up_arrow);
  lcd.createChar(3, full);

  pinMode(back_button, INPUT_PULLUP);
  pinMode(up_button, INPUT_PULLUP);
  pinMode(down_button, INPUT_PULLUP);
  pinMode(confirm_button, INPUT_PULLUP);

  plant_defined = EEPROM.read(plant_defined_address);
  plant_type = EEPROM.read(plant_type_address);
  water_time = EEPROM.read(water_time_address);
}

void loop() {

  if (start) {
    startScreen();
  }

  if (!date_defined) {
    dateDefinition();
  }

  if (!plant_defined) {
    mainMenu();

    if (choose_plant) {
      plant_type = plantSelection();
    }
    if (choose_date) {
      dateDefinition();
    }
    if (choose_reset) {
      resetMenu();
    }
  } else {
    plantScreen(plant_type);
  }
}

void plantScreen(int plant_type) {
  //Definição dos Parâmetros
  float active_high_celsius;
  float active_low_celsius;
  float active_high_light;
  float active_low_light;
  float active_water_frequency;

  //Atribuição de Valores aos Parâmetros
  if (plant_type == 1) {
    active_high_celsius = orquidea_high_celsius;
    active_low_celsius = orquidea_low_celsius;
    active_high_light = orquidea_high_light;
    active_low_light = orquidea_low_light;
    active_water_frequency = orquidea_water_frequency;
  }
  if (plant_type == 2) {
    active_high_celsius = suculenta_high_celsius;
    active_low_celsius = suculenta_low_celsius;
    active_high_light = suculenta_high_light;
    active_low_light = suculenta_low_light;
    active_water_frequency = suculenta_water_frequency;
  }
  if (plant_type == 3) {
    active_high_celsius = bromelia_high_celsius;
    active_low_celsius = bromelia_low_celsius;
    active_high_light = bromelia_high_light;
    active_low_light = bromelia_low_light;
    active_water_frequency = bromelia_water_frequency;
  }
  if (plant_type == 4) {
    active_high_celsius = espada_high_celsius;
    active_low_celsius = espada_low_celsius;
    active_high_light = espada_high_light;
    active_low_light = espada_low_light;
    active_water_frequency = espada_water_frequency;
  }
  if (plant_type == 5) {
    active_high_celsius = fitonia_high_celsius;
    active_low_celsius = fitonia_low_celsius;
    active_high_light = fitonia_high_light;
    active_low_light = fitonia_low_light;
    active_water_frequency = fitonia_water_frequency;
  }

  //Código de Funcionamento
  while (true) {
    //Atualização dos Sensores
    if (millis() - sensor_millis >= sensor_delay) {
      value_ldr_1 = analogRead(ldr_1);
      value_ldr_2 = analogRead(ldr_2);
      value_ldr_mean = ((value_ldr_1 + value_ldr_2) / 2);

      celsius_temperature = myTMP36.getTempC();

      humidity = analogRead(hygrometer);

      sensor_millis = millis();
    }

    if (millis() - clock_millis >= clock_delay) { //Relógio da tela

      current_time = now();
      if ((hour(current_time)) < 10) {
        lcd.setCursor(15, 0);
        lcd.print(0);
        lcd.setCursor(16, 0);
        lcd.print(hour(current_time));
      } else {
        lcd.setCursor(15, 0);
        lcd.print(hour(current_time));
      }
      lcd.setCursor(17, 0);
      lcd.print(":");
      if ((minute(current_time)) < 10) {
        lcd.setCursor(18, 0);
        lcd.print(0);
        lcd.setCursor(19, 0);
        lcd.print(minute(current_time));
      } else {
        lcd.setCursor(18, 0);
        lcd.print(minute(current_time));
      }

      clock_millis = millis();
    }

    day_check = (((hour(current_time)) >= 7) && (hour(current_time) < 18));

    plant_healthy = !((celsius_state != 1) || ((light_state != 1) && (day_check)) || (water_state != 1));
    
    if (plant_healthy) {
      lcd.setCursor(6, 3);
      lcd.print(" ");
      lcd.setCursor(13, 3);
      lcd.print(" ");
      lcd.setCursor(8, 0);
      lcd.write(3);
      lcd.setCursor(11, 0);
      lcd.write(3);
      lcd.setCursor(6, 1);
      lcd.write(3);
      lcd.setCursor(13, 1);
      lcd.write(3);

      for (int i = 6; i < 14; i++) {
        lcd.setCursor(i, 2);
        lcd.write(3);
      }

      // Gambiarra De Limpeza
      lcd.setCursor(0, 0);
      lcd.print("    ");
      lcd.setCursor(0, 3);
      lcd.print("    ");
      lcd.setCursor(17, 3);
      lcd.print("   ");
      lcd.setCursor(0, 2);
      lcd.print(" ");
      lcd.setCursor(3, 2);
      lcd.print(" ");
      lcd.setCursor(17, 2);
      lcd.print(" ");
      lcd.setCursor(19, 2);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.setCursor(3, 1);
      lcd.print(" ");
    }

    if (!plant_healthy) {
      lcd.setCursor(6, 1);
      lcd.print(" ");
      lcd.setCursor(13, 1);
      lcd.print(" ");
      lcd.setCursor(8, 0);
      lcd.write(3);
      lcd.setCursor(11, 0);
      lcd.write(3);
      lcd.setCursor(6, 3);
      lcd.write(3);
      lcd.setCursor(13, 3);
      lcd.write(3);

      for (int i = 6; i < 14; i++) {
        lcd.setCursor(i, 2);
        lcd.write(3);
      }

      lcd.setCursor(0, 0);
      lcd.print("AGUA");
      lcd.setCursor(0, 3);
      lcd.print("TEMP");
      lcd.setCursor(17, 3);
      lcd.print("LUZ");

      if (celsius_state == 2) { 
        lcd.setCursor(0, 2);
        lcd.write(byte(2));
        lcd.setCursor(3, 2);
        lcd.print(" ");
      }
      if (celsius_state == 1) {
        lcd.setCursor(0, 2);
        lcd.print(" ");
        lcd.setCursor(3, 2);
        lcd.print(" ");
      }
      if (celsius_state == 0) {
        lcd.setCursor(3, 2);
        lcd.write(byte(1));
        lcd.setCursor(0, 2);
        lcd.print(" ");
      }

      if (day_check) {
        if (light_state == 2) {
          lcd.setCursor(17, 2);
          lcd.write(byte(2));
          lcd.setCursor(19, 2);
          lcd.print(" ");
        }
        if (light_state == 1) {
          lcd.setCursor(17, 2);
          lcd.print(" ");
          lcd.setCursor(19, 2);
          lcd.print(" ");
        }
        if (light_state == 0) {
          lcd.setCursor(19, 2);
          lcd.write(byte(1));
          lcd.setCursor(17, 2);
          lcd.print(" ");
        }
      }

      if (water_state == 2) {
        lcd.setCursor(0, 1);
        lcd.write(byte(2));
        lcd.setCursor(3, 1);
        lcd.print(" ");
      }
      if (water_state == 1) {
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.setCursor(3, 1);
        lcd.print(" ");
      }
      if (water_state == 0) {
        lcd.setCursor(3, 1);
        lcd.write(byte(1));
        lcd.setCursor(0, 1);
        lcd.print(" ");
      }
    }

    //AVERIGUAÇÃO

    if (celsius_temperature > active_high_celsius) {  // Conferência de Temperatura
      celsius_state = 2;
    }
    if (celsius_temperature < active_low_celsius) {
      celsius_state = 0;
    }
    if ((celsius_temperature > active_low_celsius) && (celsius_temperature < active_high_celsius)) {
      celsius_state = 1;
    }

    if (value_ldr_mean > active_high_light) {  // Conferência de Luminosidade
      light_state = 2;
    }
    if (value_ldr_mean < active_low_light) {
      light_state = 0;
    }
    if ((value_ldr_mean > active_low_light) && (value_ldr_mean < active_high_light)) {
      light_state = 1;
    }

    if ((humidity < watered_metter) && (auxiliar_water)) {  //Registro de Irrigação

      water_time = current_time + (active_water_frequency * 3600);

      auxiliar_water = false;
    }
    if ((humidity >= (watered_metter + 50)) {
      auxiliar_water = true;
    }

    if (current_time >= water_time) {  //Conferência de Irrigação
      water_state = 0;
    }
    if (current_time < water_time && (humidity > flooding_metter)) {
      water_state = 1;
    }
    if (humidity <= flooding_metter) {
      water_state = 2;
    }

    if (digitalRead(back_button) == LOW) {  //Voltar ao Menu Principal

      plant_defined = false;

      auxiliar = true;

      start = false;

      while (digitalRead(back_button) == LOW)
        ;
      lcd.clear();
      delay(150);

      break;
    }

    if (digitalRead(confirm_button) == LOW) {  //Ir para tela de conferência de irrigação

      while (digitalRead(confirm_button) == LOW)
        ;
      lcd.clear();
      delay(150);

      waterTimeScreen();
    }
  }
}

void waterTimeScreen() {
  while (true) {
    if (water_time != 0) {
      lcd.setCursor(1, 0);
      lcd.print("PROXIMA IRRIGACAO:");

      lcd.setCursor(5, 2);
      printDigits(day(water_time));
      lcd.print("/");
      printDigits(month(water_time));
      lcd.print("/");
      printDigits(year(water_time));

      lcd.setCursor(8, 3);
      printDigits(hour(water_time));
      lcd.print(":");
      printDigits(minute(water_time));

    } else {
      lcd.setCursor(3, 1);
      lcd.print("AINDA NAO FOI");
      lcd.setCursor(5, 2);
      lcd.print("IRRIGADO!");
    }
    if (digitalRead(back_button) == LOW) {  //Voltar à tela de funcionamento

      lcd.clear();

      while (digitalRead(back_button) == LOW)
        ;

      delay(150);

      break;
    }
  }
}

void printDigits(int digits) {
  if (digits < 10)
    lcd.print("0");
  lcd.print(digits);
}

void startScreen() {
  //Mensagem de Boas Vindas
  lcd.setCursor(9, 0);
  lcd.print("OI!");
  lcd.setCursor(7, 1);
  lcd.print("SOU SUA");
  lcd.setCursor(6, 2);
  lcd.print("PLANTINHA");
  lcd.setCursor(6, 3);
  lcd.print("DOMESTICA");
  delay(4000);

  lcd.clear();
}

void mainMenu() {
  //Variáveis Internas
  bool option_selected = false;

  //Tela de Seleção
  lcd.setCursor(0, 0);
  lcd.print(" O QUE VOCE DESEJA? ");
  lcd.setCursor(0, 1);
  lcd.print(" SELECIONAR PLANTA  ");
  lcd.setCursor(0, 2);
  lcd.print(" CONFIGURAR HORARIO ");
  lcd.setCursor(0, 3);
  lcd.print(" REINICIAR          ");

  arrow_position = 1;

  menuVariableReset();

  while (true) {
    // Controle da Posição da Seta
    lcd.setCursor(0, arrow_position);
    lcd.write(byte(0));

    if ((digitalRead(up_button) == LOW) && (arrow_position != 1)) {
      lcd.setCursor(0, arrow_position);
      lcd.print(" ");
      arrow_position -= 1;
      while (digitalRead(up_button) == LOW)
        ;
      delay(150);
    }

    if ((digitalRead(down_button) == LOW) && (arrow_position != 3)) {
      lcd.setCursor(0, arrow_position);
      lcd.print(" ");
      arrow_position += 1;
      while (digitalRead(down_button) == LOW)
        ;
      delay(150);
    }

    if (digitalRead(confirm_button) == LOW) {

      option_selected = true;

      while (digitalRead(confirm_button) == LOW)
        ;
      lcd.clear();
      delay(150);
    };

    //Definição da Tarefa Selecionada
    if (option_selected) {
      if (arrow_position == 1) {
        choose_plant = true;
      }
      if (arrow_position == 2) {
        choose_date = true;
      }
      if (arrow_position == 3) {
        choose_reset = true;
      }
      break;
    }
  }
}

int plantSelection() {
  int menu_section = 1;

  plant_type = 1;

  while (true) {
    if (menu_section == 1) {
      lcd.setCursor(1, 0);
      lcd.print("ESCOLHA UMA PLANTA");
      lcd.setCursor(1, 1);
      lcd.print("ORQUIDEA");
      lcd.setCursor(1, 2);
      lcd.print("SUCULENTA");
      lcd.setCursor(1, 3);
      lcd.print("BROMELIA");
      lcd.setCursor(19, 3);
      lcd.write(byte(1));

      while (true) {
        lcd.setCursor(0, plant_type);
        lcd.write(byte(0));

        if ((digitalRead(up_button) == LOW) && (plant_type != 1)) {
          lcd.setCursor(0, plant_type);
          lcd.print(" ");
          plant_type -= 1;
          while (digitalRead(up_button) == LOW)
            ;
          delay(150);
        };

        if ((digitalRead(down_button) == LOW) && (plant_type != 3)) {
          lcd.setCursor(0, plant_type);
          lcd.print(" ");
          plant_type += 1;
          while (digitalRead(down_button) == LOW)
            ;
          delay(150);
        };
        if ((digitalRead(down_button) == LOW) && (plant_type = 3)) {
          lcd.clear();
          plant_type += 1;
          menu_section = 2;
          while (digitalRead(down_button) == LOW)
            ;
          delay(150);

          break;
        };
        if (digitalRead(confirm_button) == LOW) {

          plant_defined = true;
          EEPROM.write(plant_defined_address, plant_defined);

          EEPROM.write(plant_type_address, plant_type);

          start = false;
          water_time = 0;
          auxiliar_water = true;

          while (digitalRead(confirm_button) == LOW)
            ;
          lcd.clear();
          delay(150);

          return plant_type;
        };
        if (digitalRead(back_button) == LOW) {

          start = false;

          while (digitalRead(back_button) == LOW)
            ;
          lcd.clear();
          delay(150);

          return 0;
        };
      }
    }
    if (menu_section == 2) {
      lcd.setCursor(1, 0);
      lcd.print("ESCOLHA UMA PLANTA");
      lcd.setCursor(1, 1);
      lcd.print("SAO JORGE");
      lcd.setCursor(1, 2);
      lcd.print("FITONIA");

      lcd.setCursor(19, 3);
      lcd.write(byte(2));

      while (true) {
        lcd.setCursor(0, (plant_type - 3));
        lcd.write(byte(0));

        if ((digitalRead(up_button) == LOW) && (plant_type != 4)) {
          lcd.setCursor(0, (plant_type - 3));
          lcd.print(" ");
          plant_type -= 1;
          while (digitalRead(up_button) == LOW)
            ;
          delay(150);
        };
        if ((digitalRead(down_button) == LOW) && (plant_type != 5)) {
          lcd.setCursor(0, (plant_type - 3));
          lcd.print(" ");
          plant_type += 1;
          while (digitalRead(down_button) == LOW)
            ;
          delay(150);
        };
        if ((plant_type == 4) && (digitalRead(up_button) == LOW)) {
          lcd.clear();
          plant_type -= 1;
          menu_section = 1;
          while (digitalRead(up_button) == LOW)
            ;
          delay(150);

          break;
        };

        if (digitalRead(confirm_button) == LOW) {

          plant_defined = true;
          EEPROM.write(plant_defined_address, plant_defined);

          EEPROM.write(plant_type_address, plant_type);

          start = false;
          water_time = 0;
          auxiliar_water = true;

          while (digitalRead(confirm_button) == LOW)
            ;
          lcd.clear();
          delay(150);

          return plant_type;
        };
        if (digitalRead(back_button) == LOW) {

          start = false;

          while (digitalRead(back_button) == LOW)
            ;
          lcd.clear();
          delay(150);

          return 0;
        };
      }
    }
  }
}

void dateDefinition() {
  //Definição de Variáveis Internas
  int year = 2022;
  int month = 6;
  int day = 15;
  int hour = 12;
  int minute = 30;

  int day_limit;
  bool leap_year;

  int selection_state = 1;
  bool auxiliar_date = true;

  //Execução da Seleção
  while (true) {
    if ((digitalRead(back_button) == LOW) && (selection_state > 1)) {
      if ((selection_state == 3) && (day > 28)) {
        day = 28;
      }
      selection_state -= 1;

      while (digitalRead(back_button) == LOW)
        ;
      lcd.clear();
      delay(150);
    }

    if ((digitalRead(confirm_button) == LOW) && (selection_state != 0)) {
      selection_state += 1;
      lcd.clear();
      while (digitalRead(confirm_button) == LOW)
        ;
      delay(150);
    }

    if (selection_state == 1) {
      lcd.setCursor(8, 0);
      lcd.print("ANO:");
      lcd.setCursor(8, 2);
      lcd.print(year);
      lcd.setCursor(12, 1);
      lcd.write(byte(2));
      lcd.setCursor(12, 3);
      lcd.write(byte(1));

      if ((digitalRead(up_button) == LOW) && (year < 9999)) {
        year += 1;
        while (digitalRead(up_button) == LOW)
          ;
        delay(150);
      };
      if ((digitalRead(down_button) == LOW) && (year > 0)) {
        year -= 1;
        while (digitalRead(down_button) == LOW)
          ;
        delay(150);
      };
    }
    if (selection_state == 2) {
      if (month < 10) {
        lcd.setCursor(8, 0);
        lcd.print("MES:");
        lcd.setCursor(9, 2);
        lcd.print(0);
        lcd.setCursor(10, 2);
        lcd.print(month);
        lcd.setCursor(12, 1);
        lcd.write(byte(2));
        lcd.setCursor(12, 3);
        lcd.write(byte(1));
      } else {
        lcd.setCursor(8, 0);
        lcd.print("MES:");
        lcd.setCursor(9, 2);
        lcd.print(month);
        lcd.setCursor(12, 1);
        lcd.write(byte(2));
        lcd.setCursor(12, 3);
        lcd.write(byte(1));
      }

      if ((digitalRead(up_button) == LOW) && (month < 12)) {
        month += 1;
        while (digitalRead(up_button) == LOW)
          ;
        delay(150);
      };
      if ((digitalRead(down_button) == LOW) && (month > 1)) {
        month -= 1;
        while (digitalRead(down_button) == LOW)
          ;
        delay(150);
      };
    }
    if (selection_state == 3) {
      if (day < 10) {
        lcd.setCursor(8, 0);
        lcd.print("DIA:");
        lcd.setCursor(9, 2);
        lcd.print(0);
        lcd.setCursor(10, 2);
        lcd.print(day);
        lcd.setCursor(12, 1);
        lcd.write(byte(2));
        lcd.setCursor(12, 3);
        lcd.write(byte(1));
      } else {
        lcd.setCursor(8, 0);
        lcd.print("DIA:");
        lcd.setCursor(9, 2);
        lcd.print(day);
        lcd.setCursor(12, 1);
        lcd.write(byte(2));
        lcd.setCursor(12, 3);
        lcd.write(byte(1));
      }
      if (year % 4 == 0 && year % 100 != 0 || year % 400 == 0) {
        leap_year = true;
      } else {
        leap_year = false;
      }
      if (month == 2) {
        if (leap_year) {
          day_limit = 29;
        } else {
          day_limit = 28;
        }
      }
      if ((month <= 7) && (month != 2)) {
        if (month % 2 == 0) {
          day_limit = 30;
        } else {
          day_limit = 31;
        }
      }
      if (month > 7) {
        if (month % 2 == 0) {
          day_limit = 31;
        } else {
          day_limit = 30;
        }
      }

      if ((digitalRead(up_button) == LOW) && (day < day_limit)) {
        day += 1;
        while (digitalRead(up_button) == LOW)
          ;
        delay(150);
      };
      if ((digitalRead(down_button) == LOW) && (day > 1)) {
        day -= 1;
        while (digitalRead(down_button) == LOW)
          ;
        delay(150);
      };
    }
    if (selection_state == 4) {
      if (hour < 10) {
        lcd.setCursor(8, 0);
        lcd.print("HORA:");
        lcd.setCursor(9, 2);
        lcd.print(0);
        lcd.setCursor(10, 2);
        lcd.print(hour);
        lcd.setCursor(12, 1);
        lcd.write(byte(2));
        lcd.setCursor(12, 3);
        lcd.write(byte(1));
      } else {
        lcd.setCursor(8, 0);
        lcd.print("HORA:");
        lcd.setCursor(9, 2);
        lcd.print(hour);
        lcd.setCursor(12, 1);
        lcd.write(byte(2));
        lcd.setCursor(12, 3);
        lcd.write(byte(1));
      }

      if ((digitalRead(up_button) == LOW) && (hour < 23)) {
        hour += 1;
        while (digitalRead(up_button) == LOW)
          ;
        delay(150);
      };
      if ((digitalRead(down_button) == LOW) && (hour > 0)) {
        hour -= 1;
        while (digitalRead(down_button) == LOW)
          ;
        delay(150);
      };
    }
    if (selection_state == 5) {
      if (minute < 10) {
        lcd.setCursor(7, 0);
        lcd.print("MINUTOS:");
        lcd.setCursor(9, 2);
        lcd.print(0);
        lcd.setCursor(10, 2);
        lcd.print(minute);
        lcd.setCursor(12, 1);
        lcd.write(byte(2));
        lcd.setCursor(12, 3);
        lcd.write(byte(1));
      } else {
        lcd.setCursor(7, 0);
        lcd.print("MINUTOS:");
        lcd.setCursor(9, 2);
        lcd.print(minute);
        lcd.setCursor(12, 1);
        lcd.write(byte(2));
        lcd.setCursor(12, 3);
        lcd.write(byte(1));
      }

      if ((digitalRead(up_button) == LOW) && (minute < 59)) {
        minute += 1;
        while (digitalRead(up_button) == LOW)
          ;
        delay(150);
      };
      if ((digitalRead(down_button) == LOW) && (minute > 0)) {
        minute -= 1;
        while (digitalRead(down_button) == LOW)
          ;
        delay(150);
      };
    }
    if (selection_state == 6) {
      lcd.setCursor(1, 0);
      lcd.print("CONFIRMAR?");
      lcd.setCursor(1, 1);
      lcd.print("SIM");
      lcd.setCursor(1, 2);
      lcd.print("VOLTAR");

      arrow_position = 1;

      while (true) {
        lcd.setCursor(0, arrow_position);
        lcd.write(byte(0));

        if ((digitalRead(up_button) == LOW) && (arrow_position != 1)) {
          lcd.setCursor(0, arrow_position);
          lcd.print(" ");
          arrow_position -= 1;
          while (digitalRead(up_button) == LOW)
            ;
          delay(150);
        };
        if ((digitalRead(down_button) == LOW) && (arrow_position != 2)) {
          lcd.setCursor(0, arrow_position);
          lcd.print(" ");
          arrow_position += 1;
          while (digitalRead(down_button) == LOW)
            ;
          delay(150);
        };

        if ((digitalRead(confirm_button) == LOW) && (arrow_position == 1)) {
          setTime(hour, minute, 0, day, month, year);  //(ANO), (MÊS), (day), (HORA), (MINUTOS), (SEGUNDOS)

          date_defined = true;

          auxiliar_date = false;

          water_time = 0;
          auxiliar_water = true;

          if (auxiliar) {
            EEPROMClear(date_defined_address);

            start = true;
            plant_defined = false;
          }
          while (digitalRead(confirm_button) == LOW)
            ;
          lcd.clear();
          delay(150);

          break;
        };
        if ((digitalRead(confirm_button) == LOW) && (arrow_position == 2) || (digitalRead(back_button) == LOW)) {
          selection_state -= 1;
          while ((digitalRead(back_button) == LOW) || (digitalRead(confirm_button) == LOW))
            ;
          lcd.clear();
          delay(150);

          break;
        };
      }
      if (!auxiliar_date) {
        break;
      }
    }
  }
}

void resetMenu() {
  lcd.setCursor(1, 0);
  lcd.print("TEM CERTEZA?");
  lcd.setCursor(1, 1);
  lcd.print("REINICIAR");
  lcd.setCursor(1, 2);
  lcd.print("VOLTAR");

  arrow_position = 1;

  while (true) {
    lcd.setCursor(0, arrow_position);
    lcd.write(byte(0));

    if ((digitalRead(up_button) == LOW) && (arrow_position != 1)) {
      lcd.setCursor(0, arrow_position);
      lcd.print(" ");
      arrow_position -= 1;
      while (digitalRead(up_button) == LOW)
        ;
      delay(150);
    };
    if ((digitalRead(down_button) == LOW) && (arrow_position != 2)) {
      lcd.setCursor(0, arrow_position);
      lcd.print(" ");
      arrow_position += 1;
      while (digitalRead(down_button) == LOW)
        ;
      delay(150);
    };

    if ((digitalRead(confirm_button) == LOW) && (arrow_position == 1)) {

      EEPROMClear(0);

      start = true;
      date_defined = false;
      plant_defined = false;

      menuVariableReset();

      water_time = 0;
      auxiliar_water = true;

      while (digitalRead(confirm_button) == LOW)
        ;
      lcd.clear();
      delay(150);

      break;
    };

    if ((digitalRead(back_button) == LOW) || ((digitalRead(confirm_button) == LOW) && (arrow_position == 2))) {

      while ((digitalRead(back_button) == LOW) || (digitalRead(confirm_button) == LOW))
        ;
      lcd.clear();
      delay(150);

      break;
    };
  }
}

void EEPROMClear(int exception) {
  for (int i = 0; i < EEPROM.length(); i++) {
    if (i != exception) {
      EEPROM.write(i, 0);
    }
  }
}

void menuVariableReset() {
  choose_reset = false;
  choose_plant = false;
  choose_date = false;
}