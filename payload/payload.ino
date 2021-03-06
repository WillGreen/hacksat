// HackSat One - https://flux.org.uk/projects/hacksat/
// HackSat Payload, by Will Green, inbox@flux.org.uk

#include <SpriteGyro.h>
#include <SpriteMag.h>
#include <SpriteRadio.h>
#include <temp.h>

// enable output for radio and/or serial
#define USERADIO 1
#define USESERIAL 0  // disable when off LaunchPad

// delay between steps in milliseconds
#define DELAY 2000

// scale factor for ITG-3200: 14.375 counts per degree
#define GYROSCALE 14.375

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

SpriteGyro gyro = SpriteGyro();
SpriteMag mag = SpriteMag();
char sbuf[40];  // sprintf buffer

void setup() 
{
    pinMode(5, OUTPUT);  // enable LED for debugging radio
    gyro.init();
    mag.init();
    mainTempCalibrate();
    mainTempRead();  // skip first reading as it may be spurious
    gyroTempRead();  //    "    " 

    randomSeed(analogRead(0));  // seed RNG from unused pin, this doesn't work at present

    if (USERADIO)
        radio.txInit();
    if (USESERIAL)
    {
        Serial.begin(9600);
        delay(DELAY);  // give us time to open serial monitor
        broadcast("London Hackspace presents...\n");
    }
}

void loop() 
{
    switch(random(0,4))
    {
        case 1:
            broadcast(ang_vel_str(sbuf));
            break;
        case 2:
            broadcast(mag_fld_str(sbuf));
            break;
        case 3:
            broadcast(temp_str(sbuf));
            break;
        default:
            sprintf(sbuf, "HackSat One - http://hack.rs - up %lus\n", millis() / 1000);
            broadcast(sbuf);
            break;
    }
    delay(DELAY);
}

char *ang_vel_str(char *buffer)  // return a string with the angular velocity components av(x,y,z) in degrees/second
{
    AngularVelocity av = gyro.read();
    int av_x_deg = av.x / GYROSCALE;
    int av_y_deg = av.y / GYROSCALE;
    int av_z_deg = av.z / GYROSCALE;
    sprintf(buffer, "av(%d,%d,%d)\n", av_x_deg, av_y_deg, av_z_deg);
    return buffer;
}

char *mag_fld_str(char *buffer)  // return a string with the magnetometer components mg(x,y,z) in µT
{
    MagneticField mf = mag.read();
    sprintf(buffer, "mg(%d,%d,%d)\n", mf.x, mf.y, mf.z);
    return buffer;
}

char *temp_str(char *buffer)  // return a string with the main and gyro temperatures in ºC
{
    int main_temp = mainTempRead();
    int gyro_temp = gyroTempRead();
    sprintf(buffer, "tp(%d,%d)\n", main_temp, gyro_temp);
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
        int transmission_time = timed_transmit(str, 0);
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
