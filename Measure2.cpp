#include <stdio.h>
#include <string.h>
#include <math.h>
#include "lcd.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "DHT.hpp"

// By default mpu6050 devices  are on bus address 0x68
static int mpu6050_addr = 0x68;

//  HC - SR04 Sonar
const uint TRIG_PIN  = 17;      
const uint ECHO_PIN  = 16;

// //  LCD
const uint RS   =    15;
const uint EN   =    14;
const uint D0   =    13;
const uint D1   =    12;
const uint D2   =    11;
const uint D3   =    10;
const uint D4   =    9;
const uint D5   =    8;
const uint D6   =    7;
const uint D7   =    6;
int lcd;

//  DHT 
const uint DHT11_PIN = 19;

uint32_t read_range()
{
  uint64_t signaloff, signalon;

  gpio_put(TRIG_PIN, 1);
  sleep_us(10);  
  gpio_put(TRIG_PIN, 0);  

  while (gpio_get(ECHO_PIN) == 0){
    signalon = time_us_64();
  } 

  while (gpio_get(ECHO_PIN) == 1){
    signaloff = time_us_64();
  } 

  uint32_t dt = signaloff - signalon;
  return dt;
}

void core1_lcd()
{
    //  init lcd.  
    lcd = lcdInit(2, 16, 8, RS, EN, D0, D1, D2, D3, D4, D5, D6, D7); 
           
    unsigned char temp[8] = {
            0b00100,
            0b01010,
            0b01010,
            0b00100,
            0b00000,
            0b00000,
            0b00000,
            0b00000};        
            
    unsigned char inch[8] = {
            0b01010,
            0b10100,
            0b10100,
            0b10100,
            0b00000,
            0b00000,
            0b00000,
            0b00000};  

    unsigned char divider[8] = {
            0b01110,
            0b01110,
            0b01110,
            0b01110,
            0b01110,
            0b01110,
            0b01110,
            0b01110};              

    lcdCharDef(lcd, 1, temp);
    lcdCharDef(lcd, 2, inch);
    lcdCharDef(lcd, 3, divider);

    float distance = 0.0, 
           temp_C = 0.0,
           mpu_temp = 0.0;

    uint32_t dht_hum = 0;
    uint32_t dht_tmp_whole = 0;
    uint32_t dht_tmp_frac = 0;
    uint32_t t_dist = 0;

    while (1)
    {                   
        if(multicore_fifo_rvalid())
            dht_hum = multicore_fifo_pop_blocking();
        if(multicore_fifo_rvalid())
            dht_tmp_whole = multicore_fifo_pop_blocking();
        if(multicore_fifo_rvalid())
            dht_tmp_frac = multicore_fifo_pop_blocking();
        if(multicore_fifo_rvalid())
            t_dist = multicore_fifo_pop_blocking();
        
        temp_C = (uint8_t)dht_tmp_whole + (uint8_t)dht_tmp_frac * 0.1;
        distance = t_dist * sqrt(temp_C + 273.15) * 0.0010025;

        // Row 1
        lcdClear(lcd);
        lcdPosition(lcd,0,0);     
        lcdPrintf(lcd ,"%-2.1fcm", distance); 
        lcdPosition(lcd,7,0);         
        lcdPutchar(lcd, 3);  
        lcdPosition(lcd,10,0);   
        lcdPrintf(lcd ,"%-2.1f",  temp_C);
        lcdPosition(lcd, 14, 0);
        lcdPutchar(lcd, 1);
        lcdPosition(lcd, 15, 0);
        lcdPutchar(lcd, 'C');

        //  Row 2
        lcdPosition(lcd,0,1);  
        lcdPrintf(lcd ,"%-2.2f", distance / 2.54);
        lcdPosition(lcd,5,1);         
        lcdPutchar(lcd, 2); 
        lcdPosition(lcd,7,1);         
        lcdPutchar(lcd, 3); 
        lcdPosition(lcd,9,1);  
        lcdPrintf(lcd ,"Hum %-d%%", (int16_t)dht_hum);

        sleep_ms(500);
        //tight_loop_contents();
    }
}

int main() {        
    // Wait for a bit for things to khappen
    sleep_ms(1000);
    stdio_init_all();

    // init HR-sc04
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
    double distance = 0.0;

    // lcd handled by core1.
    multicore_launch_core1(core1_lcd);    

    // init dht11
    DHT dht(DHT11_PIN);
    int chk;
    dht_reading result;
    
    // Wait for a bit for things to khappen
    sleep_ms(1000);

    double temp_C;

    while (1) {
        while(1)
        {
            chk = dht.readDHT11(&result);	
            if(chk == DHTLIB_OK)
            {
                if(multicore_fifo_wready())
                    multicore_fifo_push_blocking(result.humidity);
                if(multicore_fifo_wready())
                    multicore_fifo_push_blocking(result.temp_whole);
                if(multicore_fifo_wready())
                    multicore_fifo_push_blocking(result.temp_frac);
                break;
            }
        }

        uint32_t t = read_range();
        if(multicore_fifo_wready())
            multicore_fifo_push_blocking(t);

        // temp_C = result.temp_whole + result.temp_frac * 0.1;
        // distance = t * sqrt(temp_C + 273.15) * 0.0010025;

        // printf("%2.1fcm %2.1fC\n", distance,  temp_C);  
        // printf("MCU C Hum %d%\n\n", result.humidity);
 
        sleep_ms(500);
    }

    return 0;
}

