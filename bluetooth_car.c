// Bluetooth Car Control 
#ifndef CAR_CONFIG_H
#define CAR_CONFIG_H

#include <lpc21xx.h>

// Pin Definitions
#define M1_IN1 (1<<10) // Left Motor Forward
#define M1_IN2 (1<<11) // Left Motor Backward
#define M2_IN1 (1<<12) // Right Motor Forward
#define M2_IN2 (1<<13) // Right Motor Backward

// LCD Pins
#define LCD_D (0xF<<14)
#define RS (1<<21)
#define E (1<<22)

// Timer based Delays 
void delay_ms(unsigned int ms) 
{
    T0PR = 15000 - 1; // 15000 clock cycles = 1ms at 60MHz/4 (PCLK)
    T0TCR = 0x01; // Start Timer
    while(T0TC < ms); // Wait
    T0TCR = 0x03; // Reset
    T0TCR = 0x00; // Stop
}

// LCD Functions (4-bit Mode) 
void LCD_CMD(unsigned char cmd)
 {
    IOCLR0 = LCD_D | RS; // Clear data pins and select CMD register
    IOSET0 = ((cmd & 0xF0) << 10); // Send Higher Nibble (shifted to P0.14-17)
    IOSET0 = E; delay_ms(2); IOCLR0 = E; // Pulse Enable
   
    IOCLR0 = LCD_D; // Clear data pins
    IOSET0 = ((cmd & 0x0F) << 14); // Send Lower Nibble
    IOSET0 = E; delay_ms(2);
    IOCLR0 = E; // Pulse Enable
}

void LCD_DATA(unsigned char d) {
    IOCLR0 = LCD_D;
    IOSET0 = ((d & 0xF0) << 10); // Higher Nibble
    IOSET0 = RS | E; // RS=1 for Data
    delay_ms(2);
     IOCLR0 = E;

    IOCLR0 = LCD_D;
    IOSET0 = ((d & 0x0F) << 14); // Lower Nibble
    IOSET0 = RS | E;
    delay_ms(2); 
    IOCLR0 = E;
}

void LCD_INIT() {
    IODIR0 |= LCD_D | RS | E; // Set pins as output
    LCD_CMD(0x01); // Clear
    LCD_CMD(0x02); // Return Home
    LCD_CMD(0x0C); // Display ON, Cursor OFF
    LCD_CMD(0x28); // 4-bit mode, 2 lines
}

void LCD_STR(unsigned char *s) 
{
    while(*s) {
        LCD_DATA(*s++);
    }
}

void UPDATE_STATUS(unsigned char *msg) 
{
    LCD_CMD(0xC0); // Move cursor to 2nd line
    LCD_STR(" "); // Print 16 spaces to CLEAR previous text
    LCD_CMD(0xC0); // Move back to start of 2nd line
    LCD_STR(msg); // Print the new command
}

//UART for Bluetooth 
void UARTO_CONFIG() 
{
    PINSEL0 |= 0x05; // Enable TXD0 and RXD0
    U0LCR = 0x83; // 8-bit data, DLAB = 1
    U0DLL = 97; // 9600 Baud Rate
    U0DLM = 0;
    U0LCR = 0x03; // DLAB = 0
}

unsigned char UARTO_RX()
 {
    while(!(U0LSR & 0x01)); // Wait for data
    return U0RBR;
}

// Motor Logic 
void MOTAR_INIT() 
{
    IODIR0 |= M1_IN1 | M1_IN2 | M2_IN1 | M2_IN2;
}

void MOTAR_STOP() 
{
    IOCLR0 = M1_IN1 | M1_IN2 | M2_IN1 | M2_IN2;
}

void MOTAR_FORWARD() 
{
    IOSET0 = M1_IN1 | M2_IN1;
    IOCLR0 = M1_IN2 | M2_IN2;
    LCD_STR("FORWARD");
    LCD_CMD (0X01);

}

void MOTAR_BACKWARD() 
{
    IOSET0 = M1_IN2 | M2_IN2;
    IOCLR0 = M1_IN1 | M2_IN1;
    LCD_STR("BACKWARD");
    LCD_CMD (0X01);

}

void MOTAR_LEFT() 
{
    IOSET0 = M1_IN1; // 
    IOCLR0 = M1_IN2 | M2_IN1 | M2_IN2;
    LCD_STR("LEFT");
    LCD_CMD (0X01);

}

void MOTAR_RIGHT() {
    IOSET0 = M2_IN1; 
    IOCLR0 = M1_IN1 | M1_IN2 | M2_IN2;
    LCD_STR("RIGHT");
    LCD_CMD (0X01);

}

#endif

// Main Application 
int main() 
{
    unsigned char cmd;

    UARTO_CONFIG();
    LCD_INIT();
    MOTAR_INIT();
   
    LCD_CMD(0x80); // Line 1
    LCD_STR("Robot Control:");
    MOTAR_STOP();

    while(1) {
        cmd = UARTO_RX();
       
        if (cmd == 'F') 
       {
            MOTAR_FORWARD();    
        }
        else if (cmd == 'B') 
        {
            MOTAR_BACKWARD();
        }
        else if (cmd == 'L') 
        {
            MOTAR_LEFT();
        }
        else if (cmd == 'R') 
        {
            MOTAR_RIGHT();
        }
        else if (cmd == 'S') 
        {
            MOTAR_STOP();
        }
    }
}
