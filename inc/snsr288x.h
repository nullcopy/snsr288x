/**
 * snsr288x.h
 *
 * Driver library for the SNSR288x series of devices
 */

#ifndef _SNSR288X_H_
#define _SNSR288X_H_

// The system must provide a config file.
// See snsr288x_config_template.h for reference.
#include "snsr288x_config.h"
#include "snsr288x_regs.h"

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * Public Macros
 ******************************************************************************/
#define SNSR288X_ENUM_WITHIN_RANGE(prefix, val)                                \
  ((prefix##__MIN <= (val)) && (prefix##__MAX >= (val)))
#define SNSR288X_ENUM_RANGE_SIZE(prefix) (1 + (prefix##__MAX) - (prefix##__MIN))

// Size of the channel address space for each sensor channel
#define SNSR288X_CHANNEL_ADDR_RANGE_SIZE                                       \
  (1 + SNSR288X_REG__SENSOR_1_OFFSET_CURRENT_LSB__ADDR -                       \
   SNSR288X_REG__SENSOR_1_CONFIGURATION_1__ADDR)

#define SNSR288X_MAX_DAC_CODE (0x0FFFu)

// Status flags for events and IRQs
#define SNSR288X_STAT1_FLAG__PWR_RDY SNSR288X_REG__STATUS_1__PWR_RDY__MASK
#define SNSR288X_STAT1_FLAG__VDD_OOR SNSR288X_REG__STATUS_1__VDD_OOR__MASK
#define SNSR288X_STAT1_FLAG__INVALID_CFG                                       \
  SNSR288X_REG__STATUS_1__INVALID_CFG__MASK
#define SNSR288X_STAT1_FLAG__EIS_CAL_DONE                                      \
  SNSR288X_REG__STATUS_1__EIS_CAL_DONE__MASK
#define SNSR288X_STAT1_FLAG__AC_DATA_RDY                                       \
  SNSR288X_REG__STATUS_1__AC_DATA_RDY__MASK
#define SNSR288X_STAT1_FLAG__FIFO_DATA_RDY                                     \
  SNSR288X_REG__STATUS_1__FIFO_DATA_RDY__MASK
#define SNSR288X_STAT1_FLAG__A_FULL SNSR288X_REG__STATUS_1__A_FULL__MASK
#define SNSR288X_STAT2_FLAG__TEMP_DATA_HI                                      \
  SNSR288X_REG__STATUS_2__TEMP_DATA_HI__MASK
#define SNSR288X_STAT2_FLAG__TEMP_DATA_LO                                      \
  SNSR288X_REG__STATUS_2__TEMP_DATA_LO__MASK
#define SNSR288X_STAT2_FLAG__TEMP_DATA_RDY                                     \
  SNSR288X_REG__STATUS_2__TEMP_DATA_RDY__MASK
#define SNSR288X_STAT2_FLAG__SYS_ADC_DATA_HI                                   \
  SNSR288X_REG__STATUS_2__SYS_ADC_DATA_HI__MASK
#define SNSR288X_STAT2_FLAG__SYS_ADC_DATA_LO                                   \
  SNSR288X_REG__STATUS_2__SYS_ADC_DATA_LO__MASK
#define SNSR288X_STAT2_FLAG__SYS_ADC_DATA_RDY                                  \
  SNSR288X_REG__STATUS_2__SYS_ADC_DATA_RDY__MASK
#define SNSR288X_STAT3_FLAG__S1_DET SNSR288X_REG__STATUS_3__S1_DET__MASK
#define SNSR288X_STAT3_FLAG__S1_ADC_DATA_HI                                    \
  SNSR288X_REG__STATUS_3__S1_ADC_DATA_HI__MASK
#define SNSR288X_STAT3_FLAG__S1_ADC_DATA_LO                                    \
  SNSR288X_REG__STATUS_3__S1_ADC_DATA_LO__MASK
#define SNSR288X_STAT3_FLAG__S1_ADC_DATA_RDY                                   \
  SNSR288X_REG__STATUS_3__S1_ADC_DATA_RDY__MASK
#define SNSR288X_STAT3_FLAG__S2_DET SNSR288X_REG__STATUS_3__S2_DET__MASK
#define SNSR288X_STAT3_FLAG__S2_ADC_DATA_HI                                    \
  SNSR288X_REG__STATUS_3__S2_ADC_DATA_HI__MASK
#define SNSR288X_STAT3_FLAG__S2_ADC_DATA_LO                                    \
  SNSR288X_REG__STATUS_3__S2_ADC_DATA_LO__MASK
#define SNSR288X_STAT3_FLAG__S2_ADC_DATA_RDY                                   \
  SNSR288X_REG__STATUS_3__S2_ADC_DATA_RDY__MASK
#define SNSR288X_STAT4_FLAG__S3_DET SNSR288X_REG__STATUS_4__S3_DET__MASK
#define SNSR288X_STAT4_FLAG__S3_ADC_DATA_HI                                    \
  SNSR288X_REG__STATUS_4__S3_ADC_DATA_HI__MASK
#define SNSR288X_STAT4_FLAG__S3_ADC_DATA_LO                                    \
  SNSR288X_REG__STATUS_4__S3_ADC_DATA_LO__MASK
#define SNSR288X_STAT4_FLAG__S3_ADC_DATA_RDY                                   \
  SNSR288X_REG__STATUS_4__S3_ADC_DATA_RDY__MASK
#define SNSR288X_STAT4_FLAG__S4_DET SNSR288X_REG__STATUS_4__S4_DET__MASK
#define SNSR288X_STAT4_FLAG__S4_ADC_DATA_HI                                    \
  SNSR288X_REG__STATUS_4__S4_ADC_DATA_HI__MASK
#define SNSR288X_STAT4_FLAG__S4_ADC_DATA_LO                                    \
  SNSR288X_REG__STATUS_4__S4_ADC_DATA_LO__MASK
#define SNSR288X_STAT4_FLAG__S4_ADC_DATA_RDY                                   \
  SNSR288X_REG__STATUS_4__S4_ADC_DATA_RDY__MASK
#define SNSR288X_STAT5_FLAG__GPIO1_FDET SNSR288X_REG__STATUS_5__GPIO1_FDET__MASK
#define SNSR288X_STAT5_FLAG__GPIO2_FDET SNSR288X_REG__STATUS_5__GPIO2_FDET__MASK
#define SNSR288X_STAT5_FLAG__GPIO3_FDET SNSR288X_REG__STATUS_5__GPIO3_FDET__MASK
#define SNSR288X_STAT5_FLAG__GPIO4_FDET SNSR288X_REG__STATUS_5__GPIO4_FDET__MASK
#define SNSR288X_STAT5_FLAG__GPIO1_RDET SNSR288X_REG__STATUS_5__GPIO1_RDET__MASK
#define SNSR288X_STAT5_FLAG__GPIO2_RDET SNSR288X_REG__STATUS_5__GPIO2_RDET__MASK
#define SNSR288X_STAT5_FLAG__GPIO3_RDET SNSR288X_REG__STATUS_5__GPIO3_RDET__MASK
#define SNSR288X_STAT5_FLAG__GPIO4_RDET SNSR288X_REG__STATUS_5__GPIO4_RDET__MASK

#define SNSR288X_FIFO_SIZE (256)

/*******************************************************************************
 * Public Types
 ******************************************************************************/
/**
 * \brief Assert or deassert the SPI slave select for this device
 *
 * \param select    - True => assert slave select for this device
 *                  - False => deassert slave select for this device
 *
 * \return True if successful; False if any error occurred.
 */
typedef bool (*snsr288x_spi_select_t)(bool select);

/**
 * \brief Transfer data to/from the SNSR288X chip
 * The `mosi` buffer will contain the contents to be written. Upon completion,
 * the `miso` buffer shall contain the bytes received during the transaction.
 *
 * \param mosi  - Buffer containing bytes to send to the SNSR288x device
 * \param miso  - Buffer to store bytes received from the SNSR288x device
 * \param len   - Length (in bytes) of the SPI transaction
 *
 * \return True if successful; False if any error occurred.
 */
typedef bool (*snsr288x_spi_transfer_t)(const uint8_t mosi[],
                                        uint8_t miso[],
                                        uint16_t len);

/**
 * \brief snsr288x driver error codes
 */
typedef enum
{
  SNSR288X_ERR__UNKNOWN = INT8_MIN, // Unknown error occurred
  SNSR288X_ERR__UNSUPPORTED,   // Requested operation is currently unsupported
  SNSR288X_ERR__TIMEOUT,       // Operation timed out
  SNSR288X_ERR__MODE_CONFLICT, // Current sensor configuration conflicts
                               // with the requested operation
  SNSR288X_ERR__BADARG,        // One or more of the arguments is invalid
  SNSR288X_ERR__BADCONFIG,     // The configuration is invalid

  SNSR288X_ERR__OK = 0,

  SNSR288X_ERR__MIN = SNSR288X_ERR__UNKNOWN,
  SNSR288X_ERR__MAX = SNSR288X_ERR__OK,
} snsr288x_error_t;
#define SNSR288X_ERROR_IS_VALID(err)                                           \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_ERR, err)

// Device identifiers
typedef enum
{
  SNSR288X_ID__SNSR2881 = 0x32u,
  SNSR288X_ID__SNSR2882 = 0x33u,
  SNSR288X_ID__SNSR2884 = 0x34u,

  SNSR288X_ID__MIN = SNSR288X_ID__SNSR2881,
  SNSR288X_ID__MAX = SNSR288X_ID__SNSR2884
} snsr288x_id_t;
#define SNSR288X_ID_IS_VALID(id) SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_ID, id)
#define SNSR288X_NUM_IDS SNSR288X_ENUM_RANGE_SIZE(SNSR288X_ID)

typedef enum
{
  SNSR288X_CHAN__1,
  SNSR288X_CHAN__2,
  SNSR288X_CHAN__3,
  SNSR288X_CHAN__4,

  SNSR288X_CHAN__MIN = SNSR288X_CHAN__1,
  SNSR288X_CHAN__MAX = SNSR288X_CHAN__4
} snsr288x_channel_t;
#define SNSR288X_CHAN_IS_VALID(channel)                                        \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_CHAN, channel)
#define SNSR288X_MAX_NUM_CHANNELS SNSR288X_ENUM_RANGE_SIZE(SNSR288X_CHAN)

/**
 * \brief System defined APIs to communicate with the SNSR288x device
 *
 * See function prototypes for more information about each function and its
 * implementation requirements.
 */
typedef struct
{
  snsr288x_spi_select_t spi_select;
  snsr288x_spi_transfer_t spi_transfer;
} snsr288x_phy_t;
#define SNSR288X_PHY_IS_VALID(pphy)                                            \
  ((NULL != (pphy)) && (NULL != (pphy)->spi_select) &&                         \
   (NULL != (pphy)->spi_transfer))

/**
 * \brief Driver configuration parameters
 */
typedef struct
{
  // Part ID of the connected device
  snsr288x_id_t part_id;

  // Time in milliseconds to wait before enabling data capture, after making
  // configuration changes to the device. The default value used by the Generic
  // EVK driver is 5ms, but your selection depends on the electrical
  // characteristics of your sensor electrode design.
  uint32_t settle_time_ms;
} snsr288x_driver_cfg_t;
#define SNSR288X_DRIVER_CFG_IS_VALID(pcfg)                                     \
  ((NULL != (pcfg)) && SNSR288X_ID_IS_VALID((pcfg)->part_id))

/**
 * \brief Driver context variable containing the full driver state.
 *
 * The driver context enables the library APIs to have persistant data, as may
 * be needed for state machines, interface function pointers, or other runtime
 * data. The contents of this structure should not be modified outside of the
 * driver. Use the provided snsr288x_new_context() API to create a new driver
 * context.
 */
typedef struct
{
  // Physical interface definition
  const snsr288x_phy_t phy;

  // Driver configuration parameters
  const snsr288x_driver_cfg_t drv_cfg;

  // Scratch memory (mostly used as temporary storage of registers)
  uint8_t scratch[SNSR288X_CHANNEL_ADDR_RANGE_SIZE * SNSR288X_MAX_NUM_CHANNELS];
} snsr288x_ctx_t;
#define SNSR288X_CTX_IS_VALID(pctx)                                            \
  ((NULL != (pctx)) && SNSR288X_PHY_IS_VALID(&(pctx)->phy) &&                  \
   SNSR288X_DRIVER_CFG_IS_VALID(&(pctx)->drv_cfg))

typedef enum
{
  SNSR288X_GPIO_OCFG__OPEN_DRAIN_PULLDN,
  SNSR288X_GPIO_OCFG__PUSH_PULL_ACTIVE_HIGH,
  SNSR288X_GPIO_OCFG__PUSH_PULL_ACTIVE_LOW,
  SNSR288X_GPIO_OCFG__OPEN_DRAIN_PULLUP,

  SNSR288X_GPIO_OCFG__MIN = SNSR288X_GPIO_OCFG__OPEN_DRAIN_PULLDN,
  SNSR288X_GPIO_OCFG__MAX = SNSR288X_GPIO_OCFG__OPEN_DRAIN_PULLUP
} snsr288x_gpio_ocfg_t;
#define SNSR288X_GPIO_OCFG_IS_VALID(ocfg)                                      \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_GPIO_OCFG, ocfg)

// Structure containing all event flags from the device
typedef struct
{
  uint8_t status_1; // See SNSR288X_STAT1_FLAG__<...> definitions above
  uint8_t status_2; // See SNSR288X_STAT2_FLAG__<...> definitions above
  uint8_t status_3; // See SNSR288X_STAT3_FLAG__<...> definitions above
  uint8_t status_4; // See SNSR288X_STAT4_FLAG__<...> definitions above
  uint8_t status_5; // See SNSR288X_STAT5_FLAG__<...> definitions above
} snsr288x_events_t;
#define SNSR288X_EVENT_FLAGS_ARE_VALID(pflags) (NULL != (pflags))

typedef struct
{
  snsr288x_events_t events; // Events which trigger interrupt
  snsr288x_gpio_ocfg_t pin_cfg;
} snsr288x_irq_cfg_t;
#define SNSR288X_IRQ_CFG_IS_VALID(pcfg)                                        \
  ((NULL != (pcfg)) && (SNSR288X_EVENT_FLAGS_ARE_VALID(&(pcfg)->events)) &&    \
   (SNSR288X_GPIO_OCFG_IS_VALID((pcfg)->pin_cfg)))

typedef enum
{
  SNSR288X_DET_BIAS__200mV,
  SNSR288X_DET_BIAS__400mV,
  SNSR288X_DET_BIAS__600mV,
  SNSR288X_DET_BIAS__800mV,

  SNSR288X_DET_BIAS__MIN = SNSR288X_DET_BIAS__200mV,
  SNSR288X_DET_BIAS__MAX = SNSR288X_DET_BIAS__800mV
} snsr288x_detector_bias_t;
#define SNSR288X_DET_BIAS_IS_VALID(bias)                                       \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_DET_BIAS, bias)

typedef enum
{
  SNSR288X_CLOCK_SEL__34950Hz,
  SNSR288X_CLOCK_SEL__40960Hz,

  SNSR288X_CLOCK_SEL__MIN = SNSR288X_CLOCK_SEL__34950Hz,
  SNSR288X_CLOCK_SEL__MAX = SNSR288X_CLOCK_SEL__40960Hz
} snsr288x_clock_sel_t;
#define SNSR288X_CLOCK_SEL_IS_VALID(sel)                                       \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_CLOCK_SEL, sel)

typedef enum
{
  SNSR288X_VREF__1536mV,
  SNSR288X_VREF__2048mV,
  SNSR288X_VREF__3072mV,
  SNSR288X_VREF__4096mV,

  SNSR288X_VREF__MIN = SNSR288X_VREF__1536mV,
  SNSR288X_VREF__MAX = SNSR288X_VREF__4096mV
} snsr288x_vref_t;
#define SNSR288X_VREF_IS_VALID(vref)                                           \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_VREF, vref)

/**
 * \brief ADC conversion configuration data
 * This data is used to configure the various ADCs for data capture.
 */
typedef struct
{
  snsr288x_clock_sel_t clock_select;
  snsr288x_detector_bias_t detector_bias;
  bool shared_ca;
  bool bypass_ldo;
  bool connect_vref_to_pin;    // Connects internal reference (VREF) to REF pin
  bool use_external_reference; // Use REF pin as DAC and sysadc reference
  snsr288x_vref_t vref;
  bool reference_always_on; // Do not turn off VREF during sensor detect
} snsr288x_system_cfg_t;
#define SNSR288X_SYSTEM_CFG_IS_VALID(pcfg)                                     \
  ((NULL != (pcfg)) && SNSR288X_CLOCK_SEL_IS_VALID((pcfg)->clock_select) &&    \
   SNSR288X_DET_BIAS_IS_VALID((pcfg)->detector_bias))

typedef struct
{
  uint8_t
    low_capacity_threshold;  // A_FULL flag will be set (can be configured as
                             // interrupt source) when the remaining FIFO
                             // capacity is <= `low_capacity_threshold`.
  bool clear_status_on_read; // Clear A_FULL & FIFO_DATA_RDY status flags when
                             // the FIFO is read?
  bool
    overwrite_when_full; // Should new samples overwrite old samples when the
                         // FIFO gets full? If true: Old samples will be lost as
                         // new samples overwrite them. If false: New samples
                         // will be lost when the FIFO is completely filld.
  bool
    alert_on_thresh_only; // If true:  A_FULL flag will only set when the low
                          // capacity threshold
                          //           is crossed. The flag can be cleared by
                          //           reading the device status. The flag will
                          //           not set again, until the FIFO has been
                          //           drained, and crosses the
                          //           `low_capacity_threshold` again.
                          // If false: A_FULL flag will set repeatedly for every
                          // new sample that
                          //           arrives while the FIFO has low capacity.
                          //           The flag can be cleared by reading the
                          //           device status, but the flag will continue
                          //           to be set when new samples arrive, until
                          //           the FIFO has been drained sufficiently.
} snsr288x_fifo_cfg_t;
#define SNSR288X_FIFO_CFG_IS_VALID(pcfg) ((NULL != (pcfg)))

typedef struct
{
  uint8_t write_pointer; // Position where next sample will be written
  uint8_t read_pointer; // FIFO position where the next sample will be read from
  uint16_t data_count;  // Number of data samples in the FIFO. Max is defined by
                        // SNSR288X_FIFO_SIZE
  uint8_t overflow_count; // Number of samples which overflowed the FIFO
                          // These samples have been lost. The selection of
                          // `overwrite_when_full` determines which samples were
                          // lost when overflow occurred.
} snsr288x_fifo_status_t;
#define SNSR288X_FIFO_STATUS_IS_VALID(pstat) ((NULL != (pstat)))

typedef enum
{
  // Tags for 16 bit ADC data
  SNSR288X_SAMPLE_TAG__U16__S1_DC_CURRENT = 0x00u,
  SNSR288X_SAMPLE_TAG__U16__S2_DC_CURRENT,
  SNSR288X_SAMPLE_TAG__U16__S3_DC_CURRENT,
  SNSR288X_SAMPLE_TAG__U16__S4_DC_CURRENT,
  SNSR288X_SAMPLE_TAG__U16__S1_EIS_REAL_CV_SWV,
  SNSR288X_SAMPLE_TAG__U16__S2_EIS_REAL_CV_SWV,
  SNSR288X_SAMPLE_TAG__U16__S3_EIS_REAL_CV_SWV,
  SNSR288X_SAMPLE_TAG__U16__S4_EIS_REAL_CV_SWV,
  SNSR288X_SAMPLE_TAG__U16__S1_EIS_IMAGINARY,
  SNSR288X_SAMPLE_TAG__U16__S2_EIS_IMAGINARY,
  SNSR288X_SAMPLE_TAG__U16__S3_EIS_IMAGINARY,
  SNSR288X_SAMPLE_TAG__U16__S4_EIS_IMAGINARY,
  SNSR288X_SAMPLE_TAG__U16__TEMP_SENSOR,
  SNSR288X_SAMPLE_TAG__U16__MIN = SNSR288X_SAMPLE_TAG__U16__S1_DC_CURRENT,
  SNSR288X_SAMPLE_TAG__U16__MAX = SNSR288X_SAMPLE_TAG__U16__TEMP_SENSOR,

  // Tags for 12 bit ADC data
  SNSR288X_SAMPLE_TAG__U12__S1_WORKING_AMP_OUTPUT = 0xD0u,
  SNSR288X_SAMPLE_TAG__U12__S1_WORKING_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S1_REFERENCE_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S1_COUNTER_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S2_WORKING_AMP_OUTPUT,
  SNSR288X_SAMPLE_TAG__U12__S2_WORKING_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S2_REFERENCE_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S2_COUNTER_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S3_WORKING_AMP_OUTPUT,
  SNSR288X_SAMPLE_TAG__U12__S3_WORKING_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S3_REFERENCE_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S3_COUNTER_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S4_WORKING_AMP_OUTPUT,
  SNSR288X_SAMPLE_TAG__U12__S4_WORKING_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S4_REFERENCE_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__S4_COUNTER_ELECTRODE_PIN,
  SNSR288X_SAMPLE_TAG__U12__VDD_SUPPLY_VOLTAGE,
  SNSR288X_SAMPLE_TAG__U12__GROUND_REFERENCE,
  SNSR288X_SAMPLE_TAG__U12__EXT_ANALOG_IN_0 = 0xF0u,
  SNSR288X_SAMPLE_TAG__U12__EXT_ANALOG_IN_1,
  SNSR288X_SAMPLE_TAG__U12__EXT_ANALOG_IN_2,
  SNSR288X_SAMPLE_TAG__U12__EXT_ANALOG_IN_3,
  SNSR288X_SAMPLE_TAG__U12__S1_EIS_DC_CURRENT = 0xF5u,
  SNSR288X_SAMPLE_TAG__U12__S2_EIS_DC_CURRENT,
  SNSR288X_SAMPLE_TAG__U12__S3_EIS_DC_CURRENT,
  SNSR288X_SAMPLE_TAG__U12__S4_EIS_DC_CURRENT,
  SNSR288X_SAMPLE_TAG__U12__INVALID_DATA = 0xFE,
  SNSR288X_SAMPLE_TAG__U12__MIN =
    SNSR288X_SAMPLE_TAG__U12__S1_WORKING_AMP_OUTPUT,
  SNSR288X_SAMPLE_TAG__U12__MAX = SNSR288X_SAMPLE_TAG__U12__INVALID_DATA,

  SNSR288X_SAMPLE_TAG__MIN = SNSR288X_SAMPLE_TAG__U16__MIN,
  SNSR288X_SAMPLE_TAG__MAX = SNSR288X_SAMPLE_TAG__U12__INVALID_DATA
} snsr288x_sample_tag_t;
#define SNSR288X_SAMPLE_TAG_IS_VALID(tag)                                      \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_SAMPLE_TAG, tag)

// Sample data has special encoding. Use the following macros to decode each
// sample
#pragma pack(push, 1)
typedef struct
{
  uint8_t bytes[3];
} snsr288x_fifo_sample_t;
#pragma pack(pop)
#define SNSR288X_FIFO_SAMPLE_IS_SATURATED(psamp)                               \
  (0 != ((psamp)->bytes[0] & 0x20))
#define SNSR288X_FIFO_SAMPLE_IS_AUTO_MODE(psamp)                               \
  (0 != ((psamp)->bytes[0] & 0x10))
#define SNSR288X_FIFO_SAMPLE_IS_16BITS(psamp)                                  \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_SAMPLE_TAG__U16,                         \
                             (psamp)->bytes[0] & 0x0Fu)
#define SNSR288X_FIFO_SAMPLE_IS_12BITS(psamp)                                  \
  (!SNSR288X_FIFO_SAMPLE_IS_16BITS(psamp))
#define SNSR288X_GET_SAMPLE_TAG(psample)                                       \
  (snsr288x_sample_tag_t)(                                                     \
    (SNSR288X_FIFO_SAMPLE_IS_16BITS(psample) * ((psample)->bytes[0] & 0x0F)) + \
    (SNSR288X_FIFO_SAMPLE_IS_12BITS(psample) * ((psample)->bytes[0] & 0x0F)    \
     << 4) +                                                                   \
    (SNSR288X_FIFO_SAMPLE_IS_12BITS(psample) * ((psample)->bytes[1] & 0xF0) >> \
     4))
#define SNSR288X_GET_SAMPLE_DATA(psample)                                      \
  (uint16_t)((((psamp)->bytes[1] << 8) + ((psamp)->bytes[2])) &                \
             ((SNSR288X_FIFO_SAMPLE_IS_16BITS(psample) * 0xF000) + 0x0FFF))

typedef enum
{
  SNSR288X_DAC_SEL__DAC_A,
  SNSR288X_DAC_SEL__DAC_B,
  SNSR288X_DAC_SEL__DAC_C,
  SNSR288X_DAC_SEL__DAC_D,

  SNSR288X_DAC_SEL__MIN = SNSR288X_DAC_SEL__DAC_A,
  SNSR288X_DAC_SEL__MAX = SNSR288X_DAC_SEL__DAC_B
} snsr288x_dac_sel_t;
#define SNSR288X_DAC_SEL_IS_VALID(dac)                                         \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_DAC_SEL, dac)

typedef enum
{
  SNSR288X_DET_THRESH__20nA,
  SNSR288X_DET_THRESH__40nA,
  SNSR288X_DET_THRESH__80nA,
  SNSR288X_DET_THRESH__160nA,

  SNSR288X_DET_THRESH__MIN = SNSR288X_DET_THRESH__20nA,
  SNSR288X_DET_THRESH__MAX = SNSR288X_DET_THRESH__160nA
} snsr288x_detector_threshold_t;
#define SNSR288X_DET_THRESH_IS_VALID(thresh)                                   \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_DET_THRESH, thresh)

typedef enum
{
  SNSR288X_DC_FS_RANGE__50nA,
  SNSR288X_DC_FS_RANGE__100nA,
  SNSR288X_DC_FS_RANGE__250nA,
  SNSR288X_DC_FS_RANGE__500nA,
  SNSR288X_DC_FS_RANGE__1000nA,
  SNSR288X_DC_FS_RANGE__2000nA,

  SNSR288X_DC_FS_RANGE__MIN = SNSR288X_DC_FS_RANGE__50nA,
  SNSR288X_DC_FS_RANGE__MAX = SNSR288X_DC_FS_RANGE__2000nA
} snsr288x_dc_fs_range_t;
#define SNSR288X_DC_FS_RANGE_IS_VALID(range)                                   \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_DC_FS_RANGE, range)

typedef enum
{
  SNSR288X_EIS_FS_RANGE__4uA,
  SNSR288X_EIS_FS_RANGE__8uA,
  SNSR288X_EIS_FS_RANGE__20uA,
  SNSR288X_EIS_FS_RANGE__40uA,

  SNSR288X_EIS_FS_RANGE__MIN = SNSR288X_EIS_FS_RANGE__4uA,
  SNSR288X_EIS_FS_RANGE__MAX = SNSR288X_EIS_FS_RANGE__40uA
} snsr288x_eis_fs_range_t;
#define SNSR288X_EIS_FS_RANGE_IS_VALID(range)                                  \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_EIS_FS_RANGE, range)

typedef enum
{
  SNSR288X_DC_OFFSET__0_percent,
  SNSR288X_DC_OFFSET__10_percent,
  SNSR288X_DC_OFFSET__20_percent,
  SNSR288X_DC_OFFSET__50_percent,
  SNSR288X_DC_OFFSET__10nA,
  SNSR288X_DC_OFFSET__20nA,
  SNSR288X_DC_OFFSET__40nA,
  SNSR288X_DC_OFFSET__80nA,

  SNSR288X_DC_OFFSET__MIN = SNSR288X_DC_OFFSET__0_percent,
  SNSR288X_DC_OFFSET__MAX = SNSR288X_DC_OFFSET__80nA
} snsr288x_dc_offset_t;
#define SNSR288X_DC_OFFSET_IS_VALID(offset)                                    \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_DC_OFFSET, offset)

typedef enum
{
  SNSR288X_EIS_OFFSET__0p5,
  SNSR288X_EIS_OFFSET__0p4375,
  SNSR288X_EIS_OFFSET__0p375,
  SNSR288X_EIS_OFFSET__0p3125,
  SNSR288X_EIS_OFFSET__0p25,
  SNSR288X_EIS_OFFSET__0p1875,
  SNSR288X_EIS_OFFSET__0p125,

  SNSR288X_EIS_OFFSET__MIN = SNSR288X_EIS_OFFSET__0p5,
  SNSR288X_EIS_OFFSET__MAX = SNSR288X_EIS_OFFSET__0p125
} snsr288x_eis_offset_t;
#define SNSR288X_EIS_OFFSET_IS_VALID(offset)                                   \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_EIS_OFFSET, offset)

typedef enum
{
  SNSR288X_TIME__4K_TICKS,
  SNSR288X_TIME__8K_TICKS,
  SNSR288X_TIME__16K_TICKS,
  SNSR288X_TIME__32K_TICKS,
  SNSR288X_TIME__64K_TICKS,
  SNSR288X_TIME__128K_TICKS,
  SNSR288X_TIME__256K_TICKS,
  SNSR288X_TIME__512K_TICKS,
  SNSR288X_TIME__1024K_TICKS,
  SNSR288X_TIME__2048K_TICKS,
  SNSR288X_TIME__4096K_TICKS,
  SNSR288X_TIME__8192K_TICKS,

  SNSR288X_TIME__MIN = SNSR288X_TIME__4K_TICKS,
  SNSR288X_TIME__MAX = SNSR288X_TIME__8192K_TICKS
} snsr288x_time_t;
#define SNSR288X_TIME_IS_VALID(ticks)                                          \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_TIME, ticks)

typedef enum
{
  SNSR288X_OFFSET_CONV__INPUT_AND_OFFSET, // Measure input + offset combined
  SNSR288X_OFFSET_CONV__OFFSET_ONLY,      // Measure offset only
  SNSR288X_OFFSET_CONV__ALTERNATING, // Alternate input+offset and offset only

  SNSR288X_OFFSET_CONV__MIN = SNSR288X_OFFSET_CONV__INPUT_AND_OFFSET,
  SNSR288X_OFFSET_CONV__MAX = SNSR288X_OFFSET_CONV__ALTERNATING
} snsr288x_offset_conv_t;
#define SNSR288X_OFFSET_CONV_IS_VALID(conv)                                    \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_OFFSET_CONV, conv)

typedef enum
{
  SNSR288X_SENSOR_MODE__DC_MEAS_OR_DETECTOR,
  SNSR288X_SENSOR_MODE__EIS_MEASUREMENT,

  SNSR288X_SENSOR_MODE__MIN = SNSR288X_SENSOR_MODE__DC_MEAS_OR_DETECTOR,
  SNSR288X_SENSOR_MODE__MAX = SNSR288X_SENSOR_MODE__EIS_MEASUREMENT
} snsr288x_sensor_mode_t;
#define SNSR288X_SENSOR_MODE_IS_VALID(mode)                                    \
  SNSR288X_ENUM_WITHIN_RANGE(SNSR288X_SENSOR_MODE, mode)

typedef struct
{
  bool differential;   // Trigger on differential reading?
                       // Leave false to trigger on absolute reading
  bool nonconsecutive; // Include non-consecutive trips in count?
                       // Leave false to require consecutive trips
  uint8_t trip_count;  // Number of trips required to sound alarm
                       // Must be value between 0~3 inclusive
                       // actual # of trips = 1 + trip_count
  uint16_t threshold;  // Alarm threshold value
} snsr288x_alarm_cfg_t;
#define SNSR288X_ALARM_CFG_IS_VALID(pcfg)                                      \
  ((NULL != (pcfg)) && ((pcfg)->trip_count >= 0) && ((pcfg)->trip_count < 4))

/**
 * \brief Channel configuration structure
 * This structure describes the various configuration options available to each
 * of the sensor channels. See the register map in the datasheet for a better
 * understanding of each of these configurations.
 */
typedef struct
{
  snsr288x_dac_sel_t we_dac_mux;
  snsr288x_dac_sel_t ce_dac_mux;
  snsr288x_detector_threshold_t detector_threshold;
  snsr288x_time_t conversion_time;
  snsr288x_dc_fs_range_t full_scale;
  snsr288x_dc_offset_t offset;
  snsr288x_alarm_cfg_t alarm_low;
  snsr288x_alarm_cfg_t alarm_high;
  bool enable_chargepump;
  bool enable_chopper;
  bool enable_current_limit;
  bool offset_always_active;
  bool enable_rs;
  struct
  { // True := close, False := open
    bool swb;
    bool swa;
    bool sc;
    bool srb;
    bool sra;
    bool swo;
  } switches;
} snsr288x_channel_cfg_t;
#define SNSR288X_CHANNEL_CFG_IS_VALID(pcfg)                                    \
  ((NULL != (pcfg)) && SNSR288X_DAC_SEL_IS_VALID((pcfg)->we_dac_mux) &&        \
   SNSR288X_DAC_SEL_IS_VALID((pcfg)->ce_dac_mux) &&                            \
   SNSR288X_DET_THRESH_IS_VALID((pcfg)->detector_threshold) &&                 \
   SNSR288X_TIME_IS_VALID((pcfg)->conversion_time) &&                          \
   SNSR288X_DC_FS_RANGE_IS_VALID((pcfg)->full_scale) &&                        \
   SNSR288X_DC_OFFSET_IS_VALID((pcfg)->offset) &&                              \
   SNSR288X_ALARM_CFG_IS_VALID(&(pcfg)->alarm_low) &&                          \
   SNSR288X_ALARM_CFG_IS_VALID(&(pcfg)->alarm_high))

/**
 * \brief Configure which sensors should be capturing data
 */
typedef struct
{
  /* Primary sensor(s) configuration */
  snsr288x_sensor_mode_t
    sensor_mode; // Which sensor mode (DC, EIS, detector, etc)
  union
  {
    /* Enable any of the DC channels for data capture or as a detector */
    struct
    {
      bool enable[SNSR288X_MAX_NUM_CHANNELS]; // Enable/disable each channel
      bool
        detector[SNSR288X_MAX_NUM_CHANNELS]; // Enable detector, instead of ADC?
      snsr288x_offset_conv_t offset_conversion; // Sensor offset conversion mode
      bool auto_mode;         // Repeatedly capture at the specified period?
      snsr288x_time_t period; // Capture period (in clock ticks) for auto_mode
    } dc;

    /* Enable EIS measurement on one channel only */
    struct
    {
      bool auto_mode;         // Repeatedly capture at the specified period?
      snsr288x_time_t period; // Capture period (in clock ticks) for auto_mode
      snsr288x_channel_t channel; // Which channel to measure on
    } eis;
  } sensor;

  /* Temperature sensor configuration */
  struct
  {
    snsr288x_time_t period; // Capture period (in clock ticks) for auto_mode
  } temp;
} snsr288x_capture_cfg_t;
#define SNSR288X_CAPTURE_CFG_IS_VALID(pcfg)                                    \
  ((NULL != (pcfg)) && (SNSR288X_SENSOR_MODE_IS_VALID((pcfg)->sensor_mode)) && \
   (((pcfg)->sensor_mode != SNSR288X_SENSOR_MODE__DC_MEAS_OR_DETECTOR) ||      \
    ((SNSR288X_OFFSET_CONV_IS_VALID((pcfg)->sensor.dc.offset_conversion)) &&   \
     (SNSR288X_TIME_IS_VALID((pcfg)->sensor.dc.period)))) &&                   \
   (((pcfg)->sensor_mode != SNSR288X_SENSOR_MODE__EIS_MEASUREMENT) ||          \
    ((SNSR288X_TIME_IS_VALID((pcfg)->sensor.eis.period)) &&                    \
     (SNSR288X_CHAN_IS_VALID((pcfg)->sensor.eis.channel)))) &&                 \
   SNSR288X_TIME_IS_VALID((pcfg)->temp.period))

typedef struct
{
  bool enable;
  snsr288x_alarm_cfg_t alarm_low_cfg;  // Note: differential mode not allowed
  snsr288x_alarm_cfg_t alarm_high_cfg; // Note: differential mode not allowed
} snsr288x_temp_sensor_cfg_t;
#define SNSR288X_TEMP_SENSOR_CFG_IS_VALID(pcfg)                                \
  ((NULL != (pcfg)) && SNSR288X_ALARM_CFG_IS_VALID(&(pcfg)->alarm_low_cfg) &&  \
   SNSR288X_ALARM_CFG_IS_VALID(&(pcfg)->alarm_high_cfg) &&                     \
   (false == (pcfg)->alarm_low_cfg.differential) &&                            \
   (false == (pcfg)->alarm_high_cfg.differential))

typedef struct
{
  bool enable;
  uint16_t dac_code;
} snsr288x_dac_cfg_t;
#define SNSR288X_DAC_CFG_IS_VALID(pcfg)                                        \
  ((NULL != (pcfg)) && ((pcfg)->dac_code <= SNSR288X_MAX_DAC_CODE))

/*******************************************************************************
 * Public Functions
 ******************************************************************************/
/**
 * \brief Initialize a new driver context
 *
 * Note1:   Before the driver can be used, the device must be reset by calling
 *          snsr288x_reset().
 *
 * \param phy       - System defined APIs to communicate with the SNSR288x
 * device \param drv_cfg   - system specific configuration parameters for this
 * device
 *
 * \return Returns a newly initialized context structure
 */
snsr288x_ctx_t
snsr288x_new_context(snsr288x_phy_t phy, snsr288x_driver_cfg_t drv_cfg);

/**
 * \brief Get the device identifier
 *
 * \param ctx   - Driver context, created by snsr288x_new_context(...)
 *
 * \return Returns SNSR288X_ID__UNRECOGNIZED if the device responds with an
 *       unrecognized identifier.
 */
snsr288x_id_t
snsr288x_get_device_id(snsr288x_ctx_t* ctx);

/**
 * \brief Reset the device, by executing the software reset command
 *
 * \param ctx           - Driver context, created by snsr288x_new_context(...)
 * \param timeout_ms    - Time in ms (>= 0) to wait for operation to complete
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_reset(snsr288x_ctx_t* ctx, int timeout_ms);

/**
 * \brief Read the device status registers
 *
 * \param ctx   - Driver context, created by snsr288x_new_context(...)
 *
 * \return  Returns a structure of event flags representing the current status
 */
snsr288x_events_t
snsr288x_get_status(snsr288x_ctx_t* ctx);

/**
 * \brief Read the device interrupt configuration
 *
 * \param ctx   - Driver context, created by snsr288x_new_context(...)
 *
 * \return  Returns the IRQ config structure
 */
snsr288x_irq_cfg_t
snsr288x_get_interrupt_config(snsr288x_ctx_t* ctx);

/**
 * \brief Set the device interrupt configuration
 *
 * Note:    The `SNSR288X_STAT1_FLAG__PWR_RDY` event is not available for
 *          interrupts. Setting this bit will have no effect.
 *
 * \param ctx       - Driver context, created by snsr288x_new_context(...)
 * \param irq_cfg   - Interrupt configuration
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_configure_interrupts(snsr288x_ctx_t* ctx,
                              const snsr288x_irq_cfg_t* irq_cfg);

/**
 * \brief Get the general system configuration parameters
 *
 * \param ctx       - Driver context, created by snsr288x_new_context(...)
 * \param sys_cfg   - System configuration parameters
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_get_system_config(snsr288x_ctx_t* ctx, snsr288x_system_cfg_t* sys_cfg);

/**
 * \brief Apply general system configuration parameters
 *
 * \param ctx       - Driver context, created by snsr288x_new_context(...)
 * \param sys_cfg   - System configuration parameters
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_configure_system(snsr288x_ctx_t* ctx,
                          const snsr288x_system_cfg_t* sys_cfg);

/**
 * \brief Configure the on-chip measurement sample FIFO
 *
 * \param ctx       - Driver context, created by snsr288x_new_context(...)
 * \param fifo_cfg  - FIFO configuration parameters
 * \param flush     - Flush the FIFO?
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_configure_fifo(snsr288x_ctx_t* ctx,
                        const snsr288x_fifo_cfg_t* fifo_cfg,
                        bool flush);

/**
 * \brief Read the current FIFO status
 *
 * \param ctx - Driver context, created by snsr288x_new_context(...)
 *
 * \return  Returns the FIFO status structure
 */
snsr288x_fifo_status_t
snsr288x_fifo_status(snsr288x_ctx_t* ctx);

/**
 * \brief Read sample data from the FIFO
 *
 * \param ctx       - Driver context, created by snsr288x_new_context(...)
 * \param samples   - Destination buffer to receive sample data
 * \param count     - Number of data samples to read.
 * Note: fewer samples may be read, if < `count` samples are ready in the FIFO.
 *
 * \return  Returns the number of samples read.
 *          This function will only read as many samples as are in the FIFO,
 * even if the requested `count` is larger. It may even read 0 samples if no
 * samples are in the FIFO.
 */
uint16_t
snsr288x_fifo_read(snsr288x_ctx_t* ctx,
                   snsr288x_fifo_sample_t samples[],
                   uint16_t count);

/**
 * \brief Get channel configuration
 *
 * \param ctx       - Driver context, created by snsr288x_new_context(...)
 * \param channel   - Which channel configuration to read
 * \param chan_cfg  - Destination for sensor channel configuration data
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_get_channel_config(snsr288x_ctx_t* ctx,
                            snsr288x_channel_t channel,
                            snsr288x_channel_cfg_t* chan_cfg);

/**
 * \brief Configure a sensor channel
 *
 * \param ctx       - Driver context, created by snsr288x_new_context(...)
 * \param channel   - Which channel to configure
 * \param chan_cfg  - Sensor channel configuration data to be applied
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_configure_channel(snsr288x_ctx_t* ctx,
                           snsr288x_channel_t channel,
                           const snsr288x_channel_cfg_t* chan_cfg);

/**
 * \brief Get temperature sensor configuration
 *
 * \param ctx           - Driver context, created by snsr288x_new_context(...)
 * \param temp_cfg      - Destination for temp sensor configuration data
 * \param low_trip_cnt  - Number of times alarm_low has tripped (saturating)
 * \param high_trip_cnt - Number of times alarm_high has tripped (saturating)
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_get_temp_sensor_config(snsr288x_ctx_t* ctx,
                                snsr288x_temp_sensor_cfg_t* temp_cfg,
                                uint8_t* low_trip_cnt,
                                uint8_t* high_trip_cnt);

/**
 * \brief Configure the temperature sensor
 *
 * \param ctx       - Driver context, created by snsr288x_new_context(...)
 * \param temp_cfg  - Temp sensor configuration data to be applied
 * \param reset     - Reset the alarm counters?
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_configure_temp_sensor(snsr288x_ctx_t* ctx,
                               const snsr288x_temp_sensor_cfg_t* temp_cfg,
                               bool reset);

/**
 * \brief Get the current DAC configuration
 *
 * \param ctx       - Driver context, created by snsr288x_new_context(...)
 * \param dac_sel   - Which DAC to read configuration data from
 * \param dac_cfg   - Destination for DAC configuration data
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_get_dac_config(snsr288x_ctx_t* ctx,
                        snsr288x_dac_sel_t dac_sel,
                        snsr288x_dac_cfg_t* dac_cfg);

/**
 * \brief Configure the DACs
 *
 * \param ctx       - Driver context, created by snsr288x_new_context(...)
 * \param dac_sel   - Which DAC to configure
 * \param dac_cfg   - DAC configuration data to be applied
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_configure_dac(snsr288x_ctx_t* ctx,
                       snsr288x_dac_sel_t dac_sel,
                       const snsr288x_dac_cfg_t* dac_cfg);

/**
 * \brief Start capturing data
 *
 * \param ctx           - Driver context, created by snsr288x_new_context(...)
 * \param capture_cfg   - Configure which sensors should capturing data
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_start_capture(snsr288x_ctx_t* ctx,
                       const snsr288x_capture_cfg_t* capture_cfg);

/**
 * \brief Abort any captures in process
 *
 * \param ctx   - Driver context, created by snsr288x_new_context(...)
 *
 * \return  Returns SNSR288X_ERR__OK if successful.
 *          Otherwise returns one of snsr288x_error_t codes
 */
snsr288x_error_t
snsr288x_stop_capture(snsr288x_ctx_t* ctx);

#endif /* _SNSR288X_H_ */
