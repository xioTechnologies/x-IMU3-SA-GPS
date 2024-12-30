/**
 * @file main.c
 * @author Seb Madgwick
 * @brief Main file.
 *
 * Device:
 * PIC32MM0064GPL020
 *
 * Compiler:
 * XC32 v4.45, MPLAB Harmony 3
 */

//------------------------------------------------------------------------------
// Includes

#include "definitions.h"
#include "Led/Led.h"
#include "ResetCause/ResetCause.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Timer/Timer.h"
#include "Uart/Uart1.h"
#include "Uart/Uart2.h"

//------------------------------------------------------------------------------
// Functions

int main(void) {
    SYS_Initialize(NULL);

    // Configured CLC for TRRS_RX (CLC1OUT) = U1TX
    CLC1CON = 0;
    CLC1SEL = 0;
    CLC1GLS = 0;
    CLC1CONbits.LCOE = 1; // CLCx port pin output is enabled
    CLC1SELbits.DS2 = 0b011; // UART1 TX output
    CLC1GLSbits.G1D2T = 1; // The Data Source 2 signal is enabled for Gate 1
    CLC1GLSbits.G2D2T = 1; // The Data Source 2 signal is enabled for Gate 2
    CLC1GLSbits.G3D2T = 1; // The Data Source 2 signal is enabled for Gate 3
    CLC1GLSbits.G4D2T = 1; // The Data Source 2 signal is enabled for Gate 4
    CLC1CONbits.ON = 1; // CLCx is enabled and mixing input signals

    // Configured CLC for UBLOX_RX (CLC2OUT) = U2TX
    CLC2CON = 0;
    CLC2SEL = 0;
    CLC2GLS = 0;
    CLC2CONbits.LCOE = 1; // CLCx port pin output is enabled
    CLC2SELbits.DS2 = 0b011; // UART2 TX output
    CLC2GLSbits.G1D2T = 1; // The Data Source 2 signal is enabled for Gate 1
    CLC2GLSbits.G2D2T = 1; // The Data Source 2 signal is enabled for Gate 2
    CLC2GLSbits.G3D2T = 1; // The Data Source 2 signal is enabled for Gate 3
    CLC2GLSbits.G4D2T = 1; // The Data Source 2 signal is enabled for Gate 4
    CLC2CONbits.ON = 1; // CLCx is enabled and mixing input signals

    // Initialise modules
    TimerInitialise();
    Uart1Initialise(&uartSettingsDefault);
    LedInitialise();

    // Print start up message
    TimerDelayMilliseconds(500);
    ResetCausePrint(ResetCauseGet());
    printf("x-IMU3-SA-GPS v1.0.3\n");

    // Initialise UART for 9600 baud
    UartSettings uartSettings = uartSettingsDefault;
    uartSettings.baudRate = 9600;
    Uart2Initialise(&uartSettings);
    TimerDelayMilliseconds(100);

    // Configure ublox for 15200 baud
    const uint8_t ports[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xC2, 0x01, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x7E};
    Uart2Write(ports, sizeof (ports));
    TimerDelayMilliseconds(100);

    // Initialise UART for 115200 baud
    Uart2Initialise(&uartSettingsDefault);
    TimerDelayMilliseconds(100);

    // Configure ublox for 10 Hz update rate
    const uint8_t rates[] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12};
    Uart2Write(rates, sizeof (rates));
    TimerDelayMilliseconds(100);

    // Configured CLC for TRRS_RX (CLC1OUT) = UBLOX_TX (CLCINB)
    CLC1CON = 0;
    CLC1SEL = 0;
    CLC1GLS = 0;
    CLC1CONbits.LCOE = 1; // CLCx port pin output is enabled
    CLC1SELbits.DS2 = 0b000; // CLCINB I/O pin
    CLC1GLSbits.G1D2T = 1; // The Data Source 2 signal is enabled for Gate 1
    CLC1GLSbits.G2D2T = 1; // The Data Source 2 signal is enabled for Gate 2
    CLC1GLSbits.G3D2T = 1; // The Data Source 2 signal is enabled for Gate 3
    CLC1GLSbits.G4D2T = 1; // The Data Source 2 signal is enabled for Gate 4
    CLC1CONbits.ON = 1; // CLCx is enabled and mixing input signals

    // Configured CLC for UBLOX_RX (CLC2OUT) = TRRS_TX (CLCINA)
    CLC2CON = 0;
    CLC2SEL = 0;
    CLC2GLS = 0;
    CLC2CONbits.LCOE = 1; // CLCx port pin output is enabled
    CLC2SELbits.DS1 = 0b000; // CLCINA I/O pin
    CLC2GLSbits.G1D1T = 1; // The Data Source 1 signal is enabled for Gate 1
    CLC2GLSbits.G2D1T = 1; // The Data Source 1 signal is enabled for Gate 2
    CLC2GLSbits.G3D1T = 1; // The Data Source 1 signal is enabled for Gate 3
    CLC2GLSbits.G4D1T = 1; // The Data Source 1 signal is enabled for Gate 4
    CLC2CONbits.ON = 1; // CLCx is enabled and mixing input signals

    // Main program loop
    while (true) {
        SYS_Tasks();

        // Set LED to indicate fix
        static uint64_t fixTicks;
        const uint64_t currentTicks = TimerGetTicks64();
        if ((fixTicks > 0) && (currentTicks < (fixTicks + (2 * TIMER_TICKS_PER_SECOND)))) {
            LedSet(LedModeBright);
        } else {
            LedSet(LedModeFlashing);
        }

        // Get fix value from GNGGA messages
        static char message[1024];
        static int index;
        while (Uart2GetReadAvailable() > 0) {

            // Add byte to message
            const char byte = Uart2ReadByte();
            message[index] = byte;
            if (++index >= sizeof (message)) {
                index = 0;
                continue;
            }

            // Do nothing else is message incomplete
            if (byte != '\n') {
                continue;
            }

            // Terminate string
            message[index] = '\0';
            index = 0;

            // Check message header
            const char* const header = "$GNGGA";
            if (strncmp(message, header, strlen(header)) != 0) {
                continue;
            }

            // Value begins after 6th comma
            int count = 0;
            const char* value = message;
            while (*value != '\0') {
                if (*value++ == ',') {
                    count++;
                }
                if (count == 6) {
                    break;
                }
            }

            // Parse value
            if ((*value == '1') || (*value == '2')) {
                fixTicks = currentTicks;
            }
        }
    }
    return (EXIT_FAILURE);
}

//------------------------------------------------------------------------------
// End of file
