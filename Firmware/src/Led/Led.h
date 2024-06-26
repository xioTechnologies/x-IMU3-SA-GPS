/**
 * @file Led.h
 * @author Seb Madgwick
 * @brief LED driver.
 */

#ifndef LED_H
#define LED_H

//------------------------------------------------------------------------------
// Definitions

/**
 * @brief LED mode.
 */
typedef enum {
    LedModeNormal,
    LedModeBright,
    LedModeFlashing,
} LedMode;

//------------------------------------------------------------------------------
// Function declarations

void LedInitialise(void);
void LedSet(const LedMode mode);

#endif

//------------------------------------------------------------------------------
// End of file
