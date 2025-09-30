/**
 * snsr288x_config_template.h
 *
 * Template config file for the snsr288x driver
 * The system implementer should provide a config file like this, modified
 * with system specific configuration details.
 *
 * 1) Copy this file to your include path, with the name snsr288x_config.h.
 * 2) Remove the `#if 0` and corresponding `#endif`
 * 3) Modify configuration values according to system needs
 */

#if 0 // TEMPLATE ONLY -- remove this in the actual config file

#ifndef _SNSR288X_CONFIG_H_
#define _SNSR288X_CONFIG_H_

// Provide an assert macro
#include <assert.h>
#define SNSR288X_ASSERT(condition) assert(condition)

// Provide a millisecond delay macro
#define SNSR288X_DELAY_MS(ms) OS_DELAY(ms)

// Define the maximum allowed timeout (in ms) for any functions with a timeout argument
#define SNSR288X_MAX_ALLOWED_TIMEOUT_MS (100)

#endif /* _SNSR288X_CONFIG_H_ */

#endif // TEMPLATE ONLY -- remove this in the actual config file
