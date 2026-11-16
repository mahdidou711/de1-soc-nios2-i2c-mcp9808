#include <stdio.h>
#include <stdlib.h>
#include "nios2_ctrl_reg_macro.h"
#include "montimer.h"
#include "sprint.h"
#include "I2C.h"

// constante des l'adresse des periphériques

//#define BINHEXREG  *((volatile unsigned int *) 0x80800020)
#define HEX  *((volatile unsigned int *) 0x84002000)
/*#define LEDR  *((volatile unsigned char *) ???????????)
#define SW   *((volatile unsigned char *) ???????????)
*/
#define LEDR  *((volatile unsigned short *) 0x84001000)


///////////////////////////////////////////////////////////////
/******************************************************/
//   Declaration des variables globales
/******************************************************/
//unsigned short tab_LED[9] = {0x003, 0x006, 0x00C, 0x018, 0x030, 0x060, 0x0C0, 0x081, 0x001};
unsigned short tab_LED[11] = {
    0x001,  // 0000000001
    0x003,  // 0000000011
    0x007,  // 0000000111
    0x00F,  // 0000001111
    0x01F,  // 0000011111
    0x03F,  // 0000111111
    0x07F,  // 0001111111
    0x0FF,  // 0011111111
    0x1FF,  // 0111111111
    0x3FF,  // 1111111111
    0x000   // 0000000000
};
unsigned int v1, v2;
unsigned int i = 0, j=0,jj;
volatile char Flag250ms=0;
char aff[80];
char buffer[80];

unsigned int cpt=0;
unsigned int cpt2=0;

char ack;
unsigned char temph,templ;
int temperature;

/**************************************************/
/* Interrupt routine TIMER2                       */
/**************************************************/
void Timer2_irq(void)
{
// ici on mettre le code de la routine d'interruption
        Flag250ms=1;
}

/******************************************************/
// fonction init Timer2 et son interruption
/******************************************************/
void init_IT(){
                                                        // base du TIMER2
    volatile int *Timer_ptr2 = (int *) 0x84000000; // interval timer2 base address
    // inserer dans counter la valeur de la période
    // exprimée en multiple de 20ns
    int counter =  0xBEBC20; //  multiple de 1/(50 MHz)
    *(Timer_ptr2 + 0x2) = (counter & 0xFFFF);
    *(Timer_ptr2 + 0x3) = (counter >> 16) & 0xFFFF;
    /* start interval timer, enable its interrupts */
    *(Timer_ptr2 + 1) = 0x7; // STOP = 0, START = 1, CONT = 1, ITO = 1
    /* set  mask bits to 1 (bit 0 is Nios II reset) */
    NIOS2_WRITE_IENABLE( 0x10 ); /* set interrupt mask bits for Timer2 bit4*/
    NIOS2_WRITE_STATUS( 1 ); // enable Nios II interrupts
}

///////////////////////////////////////////////////////////////
/******************************************************/
//   Fonctions   A vous
/******************************************************/

/* Lecture de la temperature du MCP9808 */
int lire_temperature(void)
{
    unsigned char haut, bas;
    int valeur;

    start_i2c();

    if (write_i2c(0x30)) {
        stop_i2c();
        return -1;
    }

    if (write_i2c(0x05)) {
        stop_i2c();
        return -1;
    }

    start_i2c();

    if (write_i2c(0x31)) {
        stop_i2c();
        return -1;
    }

    haut = read_i2c(0);
    bas = read_i2c(1);
    stop_i2c();

    valeur = ((haut & 0x0F) << 8) | bas;
    temperature = valeur * 100 / 16;

    if (haut & 0x10)
        temperature -= 25600;

    return 0;
}

/* Conversion de la temperature en BCD pour les 4 afficheurs */
unsigned int conversion_BCD(unsigned int t)
{
    unsigned int d0, d1, d2, d3;

    d0 = t % 10;
    d1 = (t / 10) % 10;
    d2 = (t / 100) % 10;
    d3 = (t / 1000) % 10;

    return (d3 << 12) | (d2 << 8) | (d1 << 4) | d0;
}


int main(){
  getperiod();
    start_timer();
    init_IT(); // initialise les interruptions du TIMER2
    v1 = getsnap();

    sprint(aff, "periode=%u", getperiod());
    puts(aff);

    HEX = 0;

    while(1){

        if (Flag250ms==1 ) {
                                Flag250ms=0;
                                LEDR = tab_LED[i++];
                                /*v2 = getsnap();
                                sprint(aff, "periode=%u", getsnapdif(v1, v2));
                                puts(aff);
                                v1 = getsnap();  */
                                i = (i >= 11) ? 0 : i;
                                cpt++;
        }



        if(cpt>3) {
                    cpt=0;
                    cpt2++;

                    if (lire_temperature() == 0) {

                        int t = temperature;
                        int negatif = (t < 0);

                        if (negatif)
                            t = -t;

                        // Affichage de la temperature sur la console
                        if (negatif)
                            sprint(buffer, "Temperature = -%u.%u%u C",
                                   (unsigned int)(t / 100),
                                   (unsigned int)((t % 100) / 10),
                                   (unsigned int)(t % 10));
                        else
                            sprint(buffer, "Temperature = %u.%u%u C",
                                   (unsigned int)(t / 100),
                                   (unsigned int)((t % 100) / 10),
                                   (unsigned int)(t % 10));

                        puts(buffer);

                        // BCD conversion and HEX display
                        if (!negatif && t <= 9999)
                            HEX = conversion_BCD((unsigned int)t);
                        else
                            HEX = 0;

                    } else {
                        puts("Erreur I2C MCP9808");
                        HEX = 0;
                    }

        }
    }
}