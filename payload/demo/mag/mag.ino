// HackSat One - https://flux.org.uk/projects/hacksat/
// Basic Magnetometer, by Will Green, inbox@flux.org.uk

#include <SpriteMag.h>
#include <SpriteRadio.h>

// enable output for radio and/or serial
#define USERADIO 0
#define USESERIAL 1  // disable when off LaunchPad

// short and long delays in milliseconds
#define SDELAY 1000
#define LDELAY 5000

// pseudorandom numbers for CDMA
unsigned char prn2[80] = {
    0b00111010, 0b00010010, 0b01111101, 0b10011010, 0b01010000, 0b10111011, 0b10101101, 0b10100111,
    0b01100110, 0b00100011, 0b01010011, 0b01001101, 0b10011110, 0b01110100, 0b00010100, 0b11101110,
    0b11010101, 0b00111110, 0b10000111, 0b00111101, 0b11101010, 0b01111111, 0b11101001, 0b01100001,
    0b00010001, 0b01100111, 0b10000000, 0b11100011, 0b11101101, 0b00101110, 0b10010000, 0b11100001,
    0b11000101, 0b11111101, 0b10010010, 0b10000001, 0b00100000, 0b11010100, 0b01001000, 0b11000001,
    0b00000110, 0b00100100, 0b01010110, 0b00001001, 0b00000010, 0b10010011, 0b01111111, 0b01000111,
    0b00001110, 0b00010010, 0b11101001, 0b01101111, 0b10001110, 0b00000011, 0b11001101, 0b00010001,
    0b00001101, 0b00101111, 0b11111100, 0b10101111, 0b01111001, 0b11000010, 0b11111001, 0b01010110,
    0b11101110, 0b01010000, 0b01011100, 0b11110011, 0b01100101, 0b10010101, 0b10001000, 0b11001101,
    0b11001011, 0b01101011, 0b10111010, 0b00010100, 0b10110011, 0b01111100, 0b10010000, 0b10111001
};
unsigned char prn3[80] = {
    0b01100010, 0b00101010, 0b11010000, 0b01000010, 0b10010001, 0b00011110, 0b00111111, 0b11010011,
    0b11101110, 0b01011000, 0b01101000, 0b01011111, 0b10110110, 0b11000100, 0b00100101, 0b10000111,
    0b11100110, 0b10010111, 0b01110011, 0b01101111, 0b01110010, 0b11010101, 0b01110101, 0b11100010,
    0b11010010, 0b00010010, 0b01111110, 0b01100110, 0b10000001, 0b01000111, 0b01010001, 0b10011100,
    0b11001000, 0b10101111, 0b10101011, 0b01111101, 0b01011110, 0b00011011, 0b01010110, 0b00111101,
    0b00001110, 0b01010100, 0b10011110, 0b00010101, 0b00000100, 0b10101000, 0b00101011, 0b10110011,
    0b00011001, 0b11010100, 0b01110101, 0b11111010, 0b01100110, 0b00000110, 0b11011110, 0b11010010,
    0b11100001, 0b01000101, 0b01010010, 0b11000100, 0b00100100, 0b11000100, 0b01011010, 0b01100000,
    0b01111001, 0b01101111, 0b01110010, 0b01001000, 0b00010111, 0b10100111, 0b10010110, 0b00100000,
    0b11010000, 0b00001110, 0b00011101, 0b11011010, 0b11110111, 0b11010010, 0b10101110, 0b11100101
};
SpriteRadio radio = SpriteRadio(prn2, prn3);  // alternate between PRNs

SpriteMag mag = SpriteMag();
char sbuf[40];

void setup() 
{
    pinMode(5, OUTPUT);  // enable LED for debugging radio
    mag.init();
    if (USERADIO)
        radio.txInit();
    if (USESERIAL)
    {
        Serial.begin(9600);
        delay(LDELAY);  // give us time to open serial monitor
    }
    broadcast("HackSat One\n");
}

void loop() 
{
    broadcast(mag_fld_str(sbuf));
    delay(LDELAY);  // wait a while before repeating loop
}

char *mag_fld_str(char *buffer)  // return a string with the magnetometer components mg(x,y,z) in µT
{
    MagneticField mf = mag.read();
    sprintf(buffer, "mg(%d,%d,%d)\n", mf.x, mf.y, mf.z);
    return buffer;
}

int heading(void)  // return an int with the heading in degrees - only uses X,Y axis
{
    MagneticField mf = mag.read();
    float heading = atan2(mf.y, mf.x);
    if(heading < 0) heading += 2 * PI;
    if(heading > 2*PI) heading -= 2 * PI;    
    return (int)(heading * (180 / PI));  // just truncates, rather than proper rounding
}

char *heading_str(char *buffer)
{
    sprintf(buffer,"%3i deg\n", heading());
    return buffer; 
}

// transmit a string and return the transmission time in ms, optionally turn on LED when transmitting
int timed_transmit(char *str, int led=0)
{
    unsigned long radio_start_time = millis();
    if (led)
        digitalWrite(5, HIGH);  // turn LED on while transmitting
    radio.transmit(str, strlen(str));
    if (led)
        digitalWrite(5, LOW);  // turn LED off
    unsigned long radio_end_time = millis();
    return radio_end_time - radio_start_time;
}

// broadcast with radio and/or serial
void broadcast(char *str)
{
    if (USESERIAL)
        Serial.print(str);
    if (USERADIO)
    {
        int transmission_time = timed_transmit(str, 1);
        if (USESERIAL)  // display information on transmission
        {  
            char time_buf[40];
            int chars_tramitted = strlen(str);
            int bits_per_second = (chars_tramitted * 8) / (transmission_time / 1000);  // just truncates, rather than proper rounding
            sprintf(time_buf, "  %d chars in %d ms (%d bps)\n", chars_tramitted, transmission_time, bits_per_second);
            Serial.print(time_buf);
        }
    }
}
