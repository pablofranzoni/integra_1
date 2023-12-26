#include <16F886.h>
#include <string.h>
#include <stdlib.h>

#fuses HS, NOWDT, NOPROTECT, BROWNOUT, PUT, NOLVP
#use delay(clock=20M) 
#use fast_io(B)
#use fast_io(C)

#use RS232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, ERRORS)

#define EEPROM_SDA PIN_C4          //Define I2C pins for EEPROM
#define EEPROM_SCL PIN_C3          //Add 2K resistor from each pin to +5V
#use I2C(MASTER, SDA=PIN_C4, SCL=PIN_C3, FAST, STREAM=DS3231_STREAM)
#include "2432.c"
#include <DS3231.c>


#define LED_PIN      PIN_C2

unsigned int serial[7]; // Array de enteros para almacenar los tokens
unsigned int saved[8];  //valores del timer - intervalo entre alarmas (6) y duracion de la activacion (2)
unsigned int received_ok; 
unsigned int i;

RTC_Time *initial;
RTC_Time *alarm;

uint8_t remain_seconds = 0;
uint8_t remain_minutes = 0;
uint8_t remain_hours   = 0;
uint8_t remain_day     = 0;
uint8_t remain_month   = 0;
uint8_t tick_alarm;
uint8_t tick_counter;
uint16_t counter;
uint16_t load_timer    = 0;
uint8_t alarm_sec      = 0;
uint8_t alarm_min      = 0;
uint8_t alarm_hour     = 0;
uint8_t alarm_day      = 0;
uint8_t alarm_month    = 0;

void Set_Next_Alarm(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

#INT_EXT // Int externa pin INT0
void ext_isr(void)
{
   tick_alarm = 1; 
   clear_interrupt(INT_EXT);
}

#INT_RDA // Int por recepción de datos serial
void serial_isr() {
    static char buffer[32];
    static int index = 0;
    char received_char;
    received_char = getc();
    
    if (received_char == ';') {
        // Procesar el campo recibido en el buffer
        // Realizar aquí las acciones necesarias con el campo completo
        // Ejemplo: Enviar el campo a través del puerto serial
        unsigned int term[3];
        unsigned int token_count = 0; // Contador de tokens
      
        strcpy(term,"|");
        char *token = strtok(buffer, term);
       
        while(token != NULL) 
        {
           serial[token_count] = atoi(token);
           token_count++;
           token = strtok(0, term);
        }
        
        if (serial[0]>0) received_ok = 1;
        else received_ok = 0;
        
        // Limpiar el buffer para el próximo campo
        index = 0;
        memset(buffer, 0, sizeof(buffer));
        
    } else if (index < sizeof(buffer) - 1) {
        buffer[index++] = received_char;
    }
}

void main() {
    set_tris_b(0xFF); 
    set_tris_c(0x00);
    
    enable_interrupts(INT_RDA);
    enable_interrupts(GLOBAL);
    enable_interrupts(INT_EXT_H2L); // with edge from high to low
    
    clear_interrupt(INT_EXT);
    delay_ms(1000);  
    
    output_low(LED_PIN); // la salida para la activacion del motor
    //leer la eeprom externa e inicializar las variables del timer
    init_ext_eeprom();
    //for(i=0; i<6; i++)
    for(i=0; i<8; i++)
    {                         
      saved[i] = read_ext_eeprom(i);
      delay_ms(150);
    }
    
    initial = RTC_Get();
    printf("TIME: %02u %02u %02u %02u %02u %02u\r\n", 
                initial->year,
                initial->month,
                initial->day, 
                initial->hours, 
                initial->minutes, 
                initial->seconds);
   
    Alarm1_IF_Reset();    // reset alarm1 interrupt flag
    Alarm2_IF_Reset();    // reset alarm1 interrupt flag
    Alarm2_Disable();     // reset alarm1 interrupt flag
    IntSqw_Set(OUT_INT);  // DS3231 INT / SQW pin configuration (interrupt when alarm)
    Disable_32kHZ();      // disable DS3231 32kHz output
   
    alarm = Alarm1_Get();
    Alarm1_Set(alarm, ONCE_PER_SECOND);
    tick_alarm = 1;
    tick_counter = 0;
    
    //alarm_year = saved[0]; //not used
    alarm_month = saved[1];
    alarm_day = saved[2];
    alarm_hour = saved[3];
    alarm_min = saved[4];
    alarm_sec = saved[5];
    load_timer = saved[6] * 60 + saved[7];
    
    counter = load_timer; //en segundos, este tiempo debe ser siempre inferior al intervalo entre alarmas
    
    printf("counter: %Lu\r\n", counter);
    //formato de envio y recepcion
    //2|23|11|3|19|0|0;
    //comando|año|mes|dia|hora|minutos|segundos; //debe terminar con ;
    //1 lectura   alarm interval 
    //2 escritura alarm interval ds3231 / ds1307
    //3 set date/time            ds3231 / ds1307
    //4 get date/time
    //5 set activ time
    //6 get activ time
   
    while (true) {
      //config parametros
      if (received_ok == 1) {

        switch (serial[0]) {
               
          case 1: 
            // 1||||||;
            printf("get interval data\r\n");
            for(i=0; i<6; i++)
            {
               printf("%02u", saved[i]);
               if (i<5) printf("|");
            }
            printf("\r\n");
            break;
                  
          case 2: 
            // 2|0|0|0|0|5|0;  //example: every 5 minutes
            printf("set interval data\r\n");
            for(i=1; i<7; i++)
            {
              saved[i-1] = serial[i];
              write_ext_eeprom(i-1, saved[i-1]);
              delay_ms(150);
            }
            
            for(i=0; i<6; i++)
            {
               printf("%02u", saved[i]);
               if (i<5) printf("|");
            }
            
            alarm_month = saved[1];
            alarm_day = saved[2];
            alarm_hour = saved[3];
            alarm_min = saved[4];
            alarm_sec = saved[5];
            
            printf("\r\n");
            break;
                  
          case 3: 
            //3|23|11|3|19|0|0;
            printf("set date/time\r\n");
            RTC_Time *initial;
            initial = RTC_Get();
            
            initial->seconds = serial[6];
            initial->minutes = serial[5];
            initial->hours = serial[4];
            initial->day = serial[3];     
            initial->month = serial[2];
            initial->year = serial[1];

            RTC_Set(initial);
            printf("\r\n");
            break;
            
          case 4:
            // 4||||||;
            printf("get date/time\r\n");
            RTC_Time *readed;
            readed = RTC_Get();
            
            printf("%02u|", readed->year);
            printf("%02u|", readed->month);
            printf("%02u|", readed->day);
            printf("%02u|", readed->hours);
            printf("%02u|", readed->minutes);
            printf("%02u", readed->seconds);

            printf("\r\n");
            break;
          
          case 5:
            printf("set activation time\r\n");
            // 5|2|30|0|0|0|0;
            for(i=6; i<8; i++)
            {
              saved[i] = serial[i-5];
              write_ext_eeprom(i, saved[i]);
              delay_ms(150);
            }
            load_timer = saved[6] * 60 + saved[7];
            counter = load_timer;
            printf("counter: %Lu\r\n", counter);
            
            for(i=6; i<8; i++)
            {
               printf("%02u", saved[i]);
               if (i<7) printf("|");
            }
            printf("\r\n");
            break;
          
          case 6:
            printf("get activation time\r\n");
            for(i=6; i<8; i++)
            {
               printf("%02u", saved[i]);
               if (i<7) printf("|");
            }
            printf("\r\n");
            break;
            
          default: 
            printf("-1|0|0|0|0|0|0\r\n"); 
            break;
         }
           
         memset(serial, 0, sizeof(serial));
         received_ok = 0;
       }
       //fin config parametros
       
       if (tick_alarm) 
       {
     
         if (Alarm1_IF_Check())
         {
            Alarm1_IF_Reset(); // reset alarm1 interrupt flag
            Set_Next_Alarm(alarm_sec, alarm_min, alarm_hour, 
                  alarm_day, alarm_month);
            counter = load_timer;
            tick_counter = 1;
            IntSqw_Set(OUT_1Hz);
         } 
         
         if (tick_counter && counter > 0) 
         {
           counter--;
           printf("modo contador\r\n");
           output_high(LED_PIN);
           
           if (counter == 0)
           {
             IntSqw_Set(OUT_INT);

             printf("modo alarma\r\n");
             output_low(LED_PIN);
           }
         } 
         
        tick_alarm = 0;
       }

       delay_ms(500);
    }
}

void Set_Next_Alarm(uint8_t alarm_sec, uint8_t alarm_min, uint8_t alarm_hour, 
                    uint8_t alarm_day, uint8_t alarm_month)
{
   initial = RTC_Get();  //hora actual
         
   alarm->seconds = initial->seconds + alarm_sec;
   alarm->minutes = initial->minutes + alarm_min;
   alarm->hours = initial->hours + alarm_hour;
   alarm->day = initial->day + alarm_day;      
   alarm->month = initial->month + alarm_month;
   alarm->year = initial->year;
   
   if (alarm->seconds > 59)
   {
      remain_seconds = alarm->seconds - 60;
      alarm->seconds = remain_seconds;
      alarm->minutes = alarm->minutes + 1;
   }
   
   if (alarm->minutes > 59)
   {
      remain_minutes = alarm->minutes - 60;
      alarm->minutes = remain_minutes;
      alarm->hours = alarm->hours + 1;
   }
   
   if (alarm->hours > 23)
   {
      remain_hours = alarm->hours - 24;
      alarm->hours = remain_hours;
      alarm->day = alarm->day + 1;
   }
   
   if (alarm->day > 30)
   {
      remain_day = alarm->day - 30;
      alarm->day = remain_day;
      alarm->month = alarm->month + 1;
   }
   
   if (alarm->month > 12)
   {
      remain_month = alarm->month - 12;
      alarm->month = remain_month;
      alarm->year = alarm->year + 1;
   }

   printf("AL1 SET R: %02u %02u %02u %02u %02u %02u\r\n", 
          alarm->year, 
          alarm->month,
          alarm->day, 
          alarm->hours, 
          alarm->minutes, 
          alarm->seconds);
   
   Alarm1_Set(alarm, DATE_HOURS_MINUTES_SECONDS_MATCH); // alarm when hours, minutes and seconds match
}
