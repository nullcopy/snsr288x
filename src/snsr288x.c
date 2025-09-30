/**
 * snsr288x.c
 *
 * Driver library for the SNSR288x series of devices
 */

#include "snsr288x.h"
#include "snsr288x_config.h"
#include "snsr288x_regs.h"

/* Std includes */
#include <stddef.h>
#include <string.h>

/*******************************************************************************
 * Private Macros
 ******************************************************************************/
#define SPI_CMD_WRITE (0x00u)
#define SPI_CMD_READ (0x80u)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif /* ARRAY_SIZE */

#ifndef MIN
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))
#endif /* MIN */

#define SNSR2881_NUM_CHANNELS (1u)
#define SNSR2882_NUM_CHANNELS (2u)
#define SNSR2884_NUM_CHANNELS (4u)

// Resolves to the number of channels in a part, for a particular snsr288x_id_t
#define NUM_CHANNELS_IN_PART(part_id)                                          \
  (((SNSR288X_ID__SNSR2881 == (part_id)) * SNSR2881_NUM_CHANNELS) +            \
   ((SNSR288X_ID__SNSR2882 == (part_id)) * SNSR2882_NUM_CHANNELS) +            \
   ((SNSR288X_ID__SNSR2884 == (part_id)) * SNSR2884_NUM_CHANNELS))

// Resolves to the first register address of the channel control registers, for
// a given snsr288x_channel_t
#define FIRST_REG_IN_CHANNEL(chan)                                             \
  (((SNSR288X_CHAN__1 == (chan)) *                                             \
    SNSR288X_REG__SENSOR_1_CONFIGURATION_1__ADDR) +                            \
   ((SNSR288X_CHAN__2 == (chan)) *                                             \
    SNSR288X_REG__SENSOR_2_CONFIGURATION_1__ADDR) +                            \
   ((SNSR288X_CHAN__3 == (chan)) *                                             \
    SNSR288X_REG__SENSOR_3_CONFIGURATION_1__ADDR) +                            \
   ((SNSR288X_CHAN__4 == (chan)) *                                             \
    SNSR288X_REG__SENSOR_4_CONFIGURATION_1__ADDR))

// Resolves to the first register address for the DAC control registers, for a
// given snsr288x_dac_sel_t
#define FIRST_REG_IN_DAC(dac)                                                  \
  (((SNSR288X_DAC_SEL__DAC_A == (dac)) * SNSR288X_REG__DACA_MSB_CODE__ADDR) +  \
   ((SNSR288X_DAC_SEL__DAC_B == (dac)) * SNSR288X_REG__DACB_MSB_CODE__ADDR) +  \
   ((SNSR288X_DAC_SEL__DAC_C == (dac)) * SNSR288X_REG__DACC_MSB_CODE__ADDR) +  \
   ((SNSR288X_DAC_SEL__DAC_D == (dac)) * SNSR288X_REG__DACD_MSB_CODE__ADDR))

#define REG_RANGE_SIZE(first_reg, last_reg)                                    \
  (1 + SNSR288X_REG__##last_reg##__ADDR - SNSR288X_REG__##first_reg##__ADDR)

#define NUMBER_OF_DAC_REGS REG_RANGE_SIZE(DACA_MSB_CODE, DACA_EN_LSB_CODE)

#define READ_REG_RANGE(dest, first_reg, last_reg)                              \
  do {                                                                         \
    spi_read(ctx,                                                              \
             SNSR288X_REG__##first_reg##__ADDR,                                \
             (dest),                                                           \
             REG_RANGE_SIZE(STATUS_1, STATUS_5));                              \
  } while (0)

#define WRITE_REG_RANGE(src, first_reg, last_reg)                              \
  do {                                                                         \
    spi_write(ctx,                                                             \
              SNSR288X_REG__##first_reg##__ADDR,                               \
              (src),                                                           \
              REG_RANGE_SIZE(STATUS_1, STATUS_5));                             \
  } while (0)

#define INIT_REGS(dest, first_reg, count)                                      \
  for (int i = 0; i < (count); i++) {                                          \
    (dest)[i] =                                                                \
      snsr288x_reg_defaults[SNSR288X_REG__##first_reg##__ADDR + i].value;      \
  }

#define INIT_REG_RANGE(dest, first_reg, last_reg)                              \
  INIT_REGS(dest, first_reg, REG_RANGE_SIZE(first_reg, last_reg))

/*******************************************************************************
 * Private Types
 ******************************************************************************/

/*******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/
static void
spi_cmd_transfer(snsr288x_ctx_t* ctx,
                 uint8_t reg,
                 bool read,
                 const uint8_t* wdata,
                 uint8_t* rdata,
                 uint16_t len);
static void
spi_read(snsr288x_ctx_t* ctx, uint8_t reg, uint8_t* data, uint16_t len);
static void
spi_write(snsr288x_ctx_t* ctx, uint8_t reg, const uint8_t* data, uint16_t len);
static uint8_t
spi_read_reg(snsr288x_ctx_t* ctx, uint8_t reg);
static void
spi_write_reg(snsr288x_ctx_t* ctx, uint8_t reg, uint8_t val);

/*******************************************************************************
 * Private Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Public API Definitions
 ******************************************************************************/
snsr288x_ctx_t
snsr288x_new_context(snsr288x_phy_t phy, snsr288x_driver_cfg_t drv_cfg)
{
  SNSR288X_ASSERT(SNSR288X_PHY_IS_VALID(&phy));
  SNSR288X_ASSERT(SNSR288X_DRIVER_CFG_IS_VALID(&drv_cfg));

  // Build context struct
  snsr288x_ctx_t ctx = {
    .phy = phy,
    .drv_cfg = drv_cfg,
  };

  // Be sure we created a valid context
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(&ctx));

  return ctx;
}

snsr288x_id_t
snsr288x_get_device_id(snsr288x_ctx_t* ctx)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  uint8_t val = spi_read_reg(ctx, SNSR288X_REG__PART_IDENTIFIER__ADDR);
  return SNSR288X_GET_FIELD(PART_IDENTIFIER, PART_ID, val);
}

snsr288x_error_t
snsr288x_reset(snsr288x_ctx_t* ctx, int timeout_ms)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  if ((timeout_ms < 0) || (timeout_ms > SNSR288X_MAX_ALLOWED_TIMEOUT_MS)) {
    return SNSR288X_ERR__BADARG;
  }

  // Set the RESET bit
  uint8_t val = SNSR288X_SET_FIELD(SYSTEM_CONTROL, RESET, 0, true);
  spi_write_reg(ctx, SNSR288X_REG__SYSTEM_CONTROL__ADDR, val);

  // Chip reset is complete when the reset bit has cleared.
  // Wait some timeout period to confirm the reset bit has cleared
  val = spi_read_reg(ctx, SNSR288X_REG__SYSTEM_CONTROL__ADDR);
  while (SNSR288X_GET_FIELD(SYSTEM_CONTROL, RESET, val) && (timeout_ms-- > 0)) {
    SNSR288X_DELAY_MS(1);
    val = spi_read_reg(ctx, SNSR288X_REG__SYSTEM_CONTROL__ADDR);
  }

  // Did we complete successfully, or timeout?
  return (timeout_ms >= 0) ? SNSR288X_ERR__OK : SNSR288X_ERR__TIMEOUT;
}

snsr288x_events_t
snsr288x_get_status(snsr288x_ctx_t* ctx)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  // Read each of the status registers
  // This will also clear any pending interrupts in the chip.
  READ_REG_RANGE(&ctx->scratch[0], STATUS_1, STATUS_5);
  return (snsr288x_events_t){
    .status_1 = ctx->scratch[0],
    .status_2 = ctx->scratch[1],
    .status_3 = ctx->scratch[2],
    .status_4 = ctx->scratch[3],
    .status_5 = ctx->scratch[4],
  };
}

snsr288x_irq_cfg_t
snsr288x_get_interrupt_config(snsr288x_ctx_t* ctx)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  // Read the INTB setup register
  uint8_t intb_setup = spi_read_reg(ctx, SNSR288X_REG__INTB_SETUP__ADDR);

  // Read each of the interrupt enable registers and return the complete config
  READ_REG_RANGE(&ctx->scratch[0], INTERRUPT_ENABLE_1, INTERRUPT_ENABLE_5);
  return (snsr288x_irq_cfg_t) {
        .events = {
            .status_1 = ctx->scratch[0],
            .status_2 = ctx->scratch[1],
            .status_3 = ctx->scratch[2],
            .status_4 = ctx->scratch[3],
            .status_5 = ctx->scratch[4],
        },
        .pin_cfg = SNSR288X_GET_FIELD(INTB_SETUP, INTB_OCFG, intb_setup),
    };
}

snsr288x_error_t
snsr288x_configure_interrupts(snsr288x_ctx_t* ctx,
                              const snsr288x_irq_cfg_t* irq_cfg)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  if (!SNSR288X_IRQ_CFG_IS_VALID(irq_cfg))
    return SNSR288X_ERR__BADARG;

  // Enable interrupt sources
  ctx->scratch[0] = irq_cfg->events.status_1;
  ctx->scratch[1] = irq_cfg->events.status_2;
  ctx->scratch[2] = irq_cfg->events.status_3;
  ctx->scratch[3] = irq_cfg->events.status_4;
  ctx->scratch[4] = irq_cfg->events.status_5;
  WRITE_REG_RANGE(&ctx->scratch[0], INTERRUPT_ENABLE_1, INTERRUPT_ENABLE_5);

  // Configure INTB pin
  uint8_t reg_val = spi_read_reg(ctx, SNSR288X_REG__INTB_SETUP__ADDR);
  reg_val = SNSR288X_SET_FIELD(
    INTB_SETUP, INTB_OCFG, reg_val, irq_cfg->pin_cfg - SNSR288X_GPIO_OCFG__MIN);
  spi_write_reg(ctx, SNSR288X_REG__INTB_SETUP__ADDR, reg_val);

  return SNSR288X_ERR__OK;
}

snsr288x_error_t
snsr288x_get_system_config(snsr288x_ctx_t* ctx, snsr288x_system_cfg_t* sys_cfg)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(NULL != sys_cfg);

  // Read the control registers
  READ_REG_RANGE(&ctx->scratch[0], SYSTEM_CONTROL, SENSOR_CONFIGURATION);
  uint8_t* const SYSTEM_CONTROL_VAL = &ctx->scratch[0];
  uint8_t* const SENSOR_CONFIGURATION_VAL = &ctx->scratch[1];

  // Decode the register values
  sys_cfg->clock_select =
    SNSR288X_GET_FIELD(SYSTEM_CONTROL, CLK_SEL, *SYSTEM_CONTROL_VAL);
  sys_cfg->detector_bias = SNSR288X_GET_FIELD(
    SENSOR_CONFIGURATION, DETECTOR_BIAS, *SENSOR_CONFIGURATION_VAL);
  sys_cfg->bypass_ldo =
    SNSR288X_GET_FIELD(SYSTEM_CONTROL, BYPASS_LDO, *SYSTEM_CONTROL_VAL);
  sys_cfg->shared_ca = SNSR288X_GET_FIELD(
    SENSOR_CONFIGURATION, SHARED_CA, *SENSOR_CONFIGURATION_VAL);

  // Read the reference control register
  uint8_t* const REFERENCE_CONTROL_VAL = &ctx->scratch[0];
  READ_REG_RANGE(&ctx->scratch[0], REFERENCE_CONTROL, REFERENCE_CONTROL);
  sys_cfg->connect_vref_to_pin =
    SNSR288X_GET_FIELD(REFERENCE_CONTROL, REF_SW, *REFERENCE_CONTROL_VAL);
  sys_cfg->use_external_reference =
    SNSR288X_GET_FIELD(REFERENCE_CONTROL, REF_MODE, *REFERENCE_CONTROL_VAL);
  sys_cfg->vref =
    SNSR288X_GET_FIELD(REFERENCE_CONTROL, REF_VAL, *REFERENCE_CONTROL_VAL);
  sys_cfg->reference_always_on =
    SNSR288X_GET_FIELD(REFERENCE_CONTROL, REF_EN, *REFERENCE_CONTROL_VAL);

  return SNSR288X_ERR__OK;
}

snsr288x_error_t
snsr288x_configure_system(snsr288x_ctx_t* ctx,
                          const snsr288x_system_cfg_t* sys_cfg)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(SNSR288X_SYSTEM_CFG_IS_VALID(sys_cfg));

  // Read the control registers
  uint8_t* const SYSTEM_CONTROL_VAL = &ctx->scratch[0];
  uint8_t* const SENSOR_CONFIGURATION_VAL = &ctx->scratch[1];
  INIT_REG_RANGE(&ctx->scratch[0], SYSTEM_CONTROL, SENSOR_CONFIGURATION);

  // Set the given control values
  *SYSTEM_CONTROL_VAL = SNSR288X_SET_FIELD(
    SYSTEM_CONTROL, CLK_SEL, *SYSTEM_CONTROL_VAL, sys_cfg->clock_select);
  *SYSTEM_CONTROL_VAL = SNSR288X_SET_FIELD(
    SYSTEM_CONTROL, BYPASS_LDO, *SYSTEM_CONTROL_VAL, sys_cfg->bypass_ldo);

  *SENSOR_CONFIGURATION_VAL = SNSR288X_SET_FIELD(SENSOR_CONFIGURATION,
                                                 SHARED_CA,
                                                 *SENSOR_CONFIGURATION_VAL,
                                                 sys_cfg->shared_ca);
  *SENSOR_CONFIGURATION_VAL = SNSR288X_SET_FIELD(SENSOR_CONFIGURATION,
                                                 DETECTOR_BIAS,
                                                 *SENSOR_CONFIGURATION_VAL,
                                                 sys_cfg->detector_bias);

  // Write the registers
  WRITE_REG_RANGE(&ctx->scratch[0], SYSTEM_CONTROL, SENSOR_CONFIGURATION);

  // Write the reference control register
  uint8_t* const REFERENCE_CONTROL_VAL = &ctx->scratch[0];
  INIT_REG_RANGE(&ctx->scratch[0], REFERENCE_CONTROL, REFERENCE_CONTROL);
  *REFERENCE_CONTROL_VAL = SNSR288X_SET_FIELD(REFERENCE_CONTROL,
                                              REF_SW,
                                              *REFERENCE_CONTROL_VAL,
                                              sys_cfg->connect_vref_to_pin);
  *REFERENCE_CONTROL_VAL = SNSR288X_SET_FIELD(REFERENCE_CONTROL,
                                              REF_MODE,
                                              *REFERENCE_CONTROL_VAL,
                                              sys_cfg->use_external_reference);
  *REFERENCE_CONTROL_VAL = SNSR288X_SET_FIELD(
    REFERENCE_CONTROL, REF_VAL, *REFERENCE_CONTROL_VAL, sys_cfg->vref);
  *REFERENCE_CONTROL_VAL = SNSR288X_SET_FIELD(REFERENCE_CONTROL,
                                              REF_EN,
                                              *REFERENCE_CONTROL_VAL,
                                              sys_cfg->reference_always_on);
  WRITE_REG_RANGE(&ctx->scratch[0], REFERENCE_CONTROL, REFERENCE_CONTROL);

  return SNSR288X_ERR__OK;
}

snsr288x_error_t
snsr288x_configure_fifo(snsr288x_ctx_t* ctx,
                        const snsr288x_fifo_cfg_t* fifo_cfg,
                        bool flush)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(SNSR288X_FIFO_CFG_IS_VALID(fifo_cfg));

  uint8_t* const FIFO_CONFIGURATION_1_VAL = &ctx->scratch[0];
  uint8_t* const FIFO_CONFIGURATION_2_VAL = &ctx->scratch[1];
  INIT_REG_RANGE(&ctx->scratch[0], FIFO_CONFIGURATION_1, FIFO_CONFIGURATION_2);

  // Set the given control values
  *FIFO_CONFIGURATION_1_VAL =
    SNSR288X_SET_FIELD(FIFO_CONFIGURATION_1,
                       FIFO_A_FULL,
                       *FIFO_CONFIGURATION_1_VAL,
                       fifo_cfg->low_capacity_threshold);

  *FIFO_CONFIGURATION_2_VAL = SNSR288X_SET_FIELD(
    FIFO_CONFIGURATION_2, FLUSH_FIFO, *FIFO_CONFIGURATION_2_VAL, flush);
  *FIFO_CONFIGURATION_2_VAL =
    SNSR288X_SET_FIELD(FIFO_CONFIGURATION_2,
                       FIFO_STAT_CLR,
                       *FIFO_CONFIGURATION_2_VAL,
                       fifo_cfg->clear_status_on_read);
  *FIFO_CONFIGURATION_2_VAL =
    SNSR288X_SET_FIELD(FIFO_CONFIGURATION_2,
                       A_FULL_TYPE,
                       *FIFO_CONFIGURATION_2_VAL,
                       fifo_cfg->alert_on_thresh_only);
  *FIFO_CONFIGURATION_2_VAL = SNSR288X_SET_FIELD(FIFO_CONFIGURATION_2,
                                                 FIFO_RO,
                                                 *FIFO_CONFIGURATION_2_VAL,
                                                 fifo_cfg->overwrite_when_full);

  // Write out the registers
  WRITE_REG_RANGE(&ctx->scratch[0], FIFO_CONFIGURATION_1, FIFO_CONFIGURATION_2);

  return SNSR288X_ERR__OK;
}

snsr288x_fifo_status_t
snsr288x_fifo_status(snsr288x_ctx_t* ctx)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  // Read the FIFO status registers
  READ_REG_RANGE(&ctx->scratch[0], FIFO_WRITE_POINTER, FIFO_COUNTER_2);
  uint8_t* const FIFO_WRITE_POINTER_VAL = &ctx->scratch[0];
  uint8_t* const FIFO_READ_POINTER_VAL = &ctx->scratch[1];
  uint8_t* const FIFO_COUNTER_1_VAL = &ctx->scratch[2];
  uint8_t* const FIFO_COUNTER_2_VAL = &ctx->scratch[3];

  // Decode the registers
  const snsr288x_fifo_status_t fifo_stat = {
    .write_pointer = *FIFO_WRITE_POINTER_VAL,
    .read_pointer = *FIFO_READ_POINTER_VAL,
    .overflow_count =
      SNSR288X_GET_FIELD(FIFO_COUNTER_1, OVF_COUNTER, *FIFO_COUNTER_1_VAL),
    .data_count =
      (SNSR288X_GET_FIELD(FIFO_COUNTER_1, FIFO_DATA_COUNT, *FIFO_COUNTER_1_VAL)
       << 8) +
      (SNSR288X_GET_FIELD(FIFO_COUNTER_2, FIFO_DATA_COUNT, *FIFO_COUNTER_2_VAL)
       << 0),
  };

  return fifo_stat;
}

uint16_t
snsr288x_fifo_read(snsr288x_ctx_t* ctx,
                   snsr288x_fifo_sample_t samples[],
                   uint16_t count)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(SNSR288X_FIFO_SIZE >= count);

  const uint16_t read_len = MIN(snsr288x_fifo_status(ctx).data_count, count);
  spi_read(ctx,
           SNSR288X_REG__FIFO_DATA__ADDR,
           (uint8_t*)samples,
           sizeof(samples[0].bytes) * read_len);
  return read_len;
}

snsr288x_error_t
snsr288x_get_channel_config(snsr288x_ctx_t* ctx,
                            snsr288x_channel_t channel,
                            snsr288x_channel_cfg_t* chan_cfg)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(NULL != chan_cfg);

  // Look up how many sensor channels this device has
  const uint8_t num_channels = NUM_CHANNELS_IN_PART(ctx->drv_cfg.part_id);
  if (!SNSR288X_CHAN_IS_VALID(channel) ||
      (num_channels < channel - SNSR288X_CHAN__MIN)) {
    return SNSR288X_ERR__BADARG;
  }

  // Read all sensor registers, except the offset current readout regs
  const uint8_t base_addr = FIRST_REG_IN_CHANNEL(channel);
  spi_read(ctx, base_addr, &ctx->scratch[0], SNSR288X_CHANNEL_ADDR_RANGE_SIZE);
  uint8_t* const CFG1_VAL = &ctx->scratch[0x0];
  uint8_t* const CFG2_VAL = &ctx->scratch[0x1];
  uint8_t* const CFG3_VAL = &ctx->scratch[0x2];
  uint8_t* const CFG4_VAL = &ctx->scratch[0x3];
  uint8_t* const CFG5_VAL = &ctx->scratch[0x4];
  uint8_t* const ALARM_LOW_SETUP_VAL = &ctx->scratch[0x5];
  uint8_t* const ALARM_HIGH_SETUP_VAL = &ctx->scratch[0x6];
  uint8_t* const ALARM_LOW_MSB_VAL = &ctx->scratch[0x7];
  uint8_t* const ALARM_LOW_LSB_VAL = &ctx->scratch[0x8];
  uint8_t* const ALARM_HIGH_MSB_VAL = &ctx->scratch[0x9];
  uint8_t* const ALARM_HIGH_LSB_VAL = &ctx->scratch[0xA];

  chan_cfg->we_dac_mux =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_1, SN_WE_DAC_MX, *CFG1_VAL);
  chan_cfg->ce_dac_mux =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_1, SN_CE_DAC_MX, *CFG1_VAL);
  chan_cfg->detector_threshold = SNSR288X_GET_FIELD(
    SENSOR_N_CONFIGURATION_3, SN_DETECTOR_THRESHOLD, *CFG3_VAL);
  chan_cfg->conversion_time =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_5, SN_CONV_TIME, *CFG5_VAL);
  chan_cfg->full_scale =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_4, SN_FSR, *CFG4_VAL);
  chan_cfg->offset =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_4, SN_OFFSET_SEL, *CFG4_VAL);
  chan_cfg->alarm_low.differential = SNSR288X_GET_FIELD(
    SENSOR_N_ALARM_LOW_SETUP, SN_LO_MODE, *ALARM_LOW_SETUP_VAL);
  chan_cfg->alarm_high.differential = SNSR288X_GET_FIELD(
    SENSOR_N_ALARM_HIGH_SETUP, SN_HI_MODE, *ALARM_HIGH_SETUP_VAL);
  chan_cfg->alarm_low.nonconsecutive = SNSR288X_GET_FIELD(
    SENSOR_N_ALARM_LOW_SETUP, SN_LO_TRIP, *ALARM_LOW_SETUP_VAL);
  chan_cfg->alarm_high.nonconsecutive = SNSR288X_GET_FIELD(
    SENSOR_N_ALARM_HIGH_SETUP, SN_HI_TRIP, *ALARM_HIGH_SETUP_VAL);
  chan_cfg->alarm_low.threshold =
    *ALARM_LOW_LSB_VAL + (((uint16_t)*ALARM_LOW_MSB_VAL) << 8);
  chan_cfg->alarm_high.threshold =
    *ALARM_HIGH_LSB_VAL + (((uint16_t)*ALARM_HIGH_MSB_VAL) << 8);
  chan_cfg->alarm_low.trip_count = SNSR288X_GET_FIELD(
    SENSOR_N_ALARM_LOW_SETUP, SN_LO_TRIP_CNT, *ALARM_LOW_SETUP_VAL);
  chan_cfg->alarm_high.trip_count = SNSR288X_GET_FIELD(
    SENSOR_N_ALARM_HIGH_SETUP, SN_HI_TRIP_CNT, *ALARM_HIGH_SETUP_VAL);
  chan_cfg->enable_chargepump =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_1, SN_CP_EN, *CFG1_VAL);
  chan_cfg->enable_chopper =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_1, SN_CHOP_EN, *CFG1_VAL);
  chan_cfg->enable_current_limit =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_2, SN_ILIM_EN, *CFG2_VAL);
  chan_cfg->offset_always_active =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_3, SN_IOS_MODE, *CFG3_VAL);
  chan_cfg->enable_rs =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_2, SN_RS, *CFG2_VAL);
  chan_cfg->switches.swb =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_2, SN_SWB, *CFG2_VAL);
  chan_cfg->switches.swa =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_2, SN_SWA, *CFG2_VAL);
  chan_cfg->switches.sc =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_2, SN_SC, *CFG2_VAL);
  chan_cfg->switches.srb =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_2, SN_SRB, *CFG2_VAL);
  chan_cfg->switches.sra =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_2, SN_SRA, *CFG2_VAL);
  chan_cfg->switches.swo =
    SNSR288X_GET_FIELD(SENSOR_N_CONFIGURATION_2, SN_SWO, *CFG2_VAL);

  return SNSR288X_ERR__OK;
}

snsr288x_error_t
snsr288x_configure_channel(snsr288x_ctx_t* ctx,
                           snsr288x_channel_t channel,
                           const snsr288x_channel_cfg_t* chan_cfg)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(SNSR288X_CHANNEL_CFG_IS_VALID(chan_cfg));

  // Look up how many sensor channels this device has
  const uint8_t num_channels = NUM_CHANNELS_IN_PART(ctx->drv_cfg.part_id);
  if (!SNSR288X_CHAN_IS_VALID(channel) ||
      (num_channels < channel - SNSR288X_CHAN__MIN)) {
    return SNSR288X_ERR__BADARG;
  }

  // Populate all sensor registers, except the offset current readout regs
  uint8_t* const CFG1_VAL = &ctx->scratch[0x0];
  uint8_t* const CFG2_VAL = &ctx->scratch[0x1];
  uint8_t* const CFG3_VAL = &ctx->scratch[0x2];
  uint8_t* const CFG4_VAL = &ctx->scratch[0x3];
  uint8_t* const CFG5_VAL = &ctx->scratch[0x4];
  uint8_t* const ALARM_LOW_SETUP_VAL = &ctx->scratch[0x5];
  uint8_t* const ALARM_HIGH_SETUP_VAL = &ctx->scratch[0x6];
  uint8_t* const ALARM_LOW_MSB_VAL = &ctx->scratch[0x7];
  uint8_t* const ALARM_LOW_LSB_VAL = &ctx->scratch[0x8];
  uint8_t* const ALARM_HIGH_MSB_VAL = &ctx->scratch[0x9];
  uint8_t* const ALARM_HIGH_LSB_VAL = &ctx->scratch[0xA];

  // Initialize scratch with default values
  *CFG1_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_CONFIGURATION_1);
  *CFG2_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_CONFIGURATION_2);
  *CFG3_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_CONFIGURATION_3);
  *CFG4_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_CONFIGURATION_4);
  *CFG5_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_CONFIGURATION_5);
  *ALARM_LOW_SETUP_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_ALARM_LOW_SETUP);
  *ALARM_HIGH_SETUP_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_ALARM_HIGH_SETUP);
  *ALARM_LOW_MSB_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_ALARM_LOW_MSB);
  *ALARM_LOW_LSB_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_ALARM_LOW_LSB);
  *ALARM_HIGH_MSB_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_ALARM_HIGH_MSB);
  *ALARM_HIGH_LSB_VAL = SNSR288X_REG_DEFAULT(SENSOR_1_ALARM_HIGH_LSB);

  *CFG1_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_1, SN_WE_DAC_MX, *CFG1_VAL, chan_cfg->we_dac_mux);
  *CFG1_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_1, SN_CE_DAC_MX, *CFG1_VAL, chan_cfg->ce_dac_mux);
  *CFG1_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_1, SN_CP_EN, *CFG1_VAL, chan_cfg->enable_chargepump);
  *CFG1_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_1, SN_CHOP_EN, *CFG1_VAL, chan_cfg->enable_chopper);

  *CFG2_VAL = SNSR288X_SET_FIELD(SENSOR_N_CONFIGURATION_2,
                                 SN_ILIM_EN,
                                 *CFG2_VAL,
                                 chan_cfg->enable_current_limit);
  *CFG2_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_2, SN_RS, *CFG2_VAL, chan_cfg->enable_rs);
  *CFG2_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_2, SN_SWB, *CFG2_VAL, chan_cfg->switches.swb);
  *CFG2_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_2, SN_SWA, *CFG2_VAL, chan_cfg->switches.swa);
  *CFG2_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_2, SN_SC, *CFG2_VAL, chan_cfg->switches.sc);
  *CFG2_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_2, SN_SRB, *CFG2_VAL, chan_cfg->switches.srb);
  *CFG2_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_2, SN_SRA, *CFG2_VAL, chan_cfg->switches.sra);
  *CFG2_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_2, SN_SWO, *CFG2_VAL, chan_cfg->switches.swo);

  *CFG3_VAL = SNSR288X_SET_FIELD(SENSOR_N_CONFIGURATION_3,
                                 SN_IOS_MODE,
                                 *CFG3_VAL,
                                 chan_cfg->offset_always_active);
  *CFG3_VAL = SNSR288X_SET_FIELD(SENSOR_N_CONFIGURATION_3,
                                 SN_DETECTOR_THRESHOLD,
                                 *CFG3_VAL,
                                 chan_cfg->detector_threshold);

  *CFG4_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_4, SN_FSR, *CFG4_VAL, chan_cfg->full_scale);
  *CFG4_VAL = SNSR288X_SET_FIELD(
    SENSOR_N_CONFIGURATION_4, SN_OFFSET_SEL, *CFG4_VAL, chan_cfg->offset);

  *CFG5_VAL = SNSR288X_SET_FIELD(SENSOR_N_CONFIGURATION_5,
                                 SN_CONV_TIME,
                                 *CFG5_VAL,
                                 chan_cfg->conversion_time);

  *ALARM_LOW_SETUP_VAL = SNSR288X_SET_FIELD(SENSOR_N_ALARM_LOW_SETUP,
                                            SN_LO_MODE,
                                            *ALARM_LOW_SETUP_VAL,
                                            chan_cfg->alarm_low.differential);
  *ALARM_LOW_SETUP_VAL = SNSR288X_SET_FIELD(SENSOR_N_ALARM_LOW_SETUP,
                                            SN_LO_TRIP,
                                            *ALARM_LOW_SETUP_VAL,
                                            chan_cfg->alarm_low.nonconsecutive);
  *ALARM_LOW_SETUP_VAL = SNSR288X_SET_FIELD(SENSOR_N_ALARM_LOW_SETUP,
                                            SN_LO_TRIP_CNT,
                                            *ALARM_LOW_SETUP_VAL,
                                            chan_cfg->alarm_low.trip_count);
  *ALARM_LOW_SETUP_VAL = SNSR288X_SET_FIELD(SENSOR_N_ALARM_LOW_SETUP,
                                            SN_RST_LO_CNTR,
                                            *ALARM_LOW_SETUP_VAL,
                                            true); // Reset the trip counter

  *ALARM_HIGH_SETUP_VAL = SNSR288X_SET_FIELD(SENSOR_N_ALARM_HIGH_SETUP,
                                             SN_HI_MODE,
                                             *ALARM_HIGH_SETUP_VAL,
                                             chan_cfg->alarm_high.differential);
  *ALARM_HIGH_SETUP_VAL =
    SNSR288X_SET_FIELD(SENSOR_N_ALARM_HIGH_SETUP,
                       SN_HI_TRIP,
                       *ALARM_HIGH_SETUP_VAL,
                       chan_cfg->alarm_high.nonconsecutive);
  *ALARM_HIGH_SETUP_VAL = SNSR288X_SET_FIELD(SENSOR_N_ALARM_HIGH_SETUP,
                                             SN_HI_TRIP_CNT,
                                             *ALARM_HIGH_SETUP_VAL,
                                             chan_cfg->alarm_high.trip_count);
  *ALARM_HIGH_SETUP_VAL = SNSR288X_SET_FIELD(SENSOR_N_ALARM_HIGH_SETUP,
                                             SN_RST_HI_CNTR,
                                             *ALARM_HIGH_SETUP_VAL,
                                             true); // Reset the trip counter

  *ALARM_LOW_MSB_VAL = 0xFF & (chan_cfg->alarm_low.threshold >> 8);
  *ALARM_LOW_LSB_VAL = 0xFF & chan_cfg->alarm_low.threshold;
  *ALARM_HIGH_MSB_VAL = 0xFF & (chan_cfg->alarm_high.threshold >> 8);
  *ALARM_HIGH_LSB_VAL = 0xFF & chan_cfg->alarm_high.threshold;

  const uint8_t base_addr = FIRST_REG_IN_CHANNEL(channel);
  spi_write(ctx, base_addr, &ctx->scratch[0], SNSR288X_CHANNEL_ADDR_RANGE_SIZE);

  return SNSR288X_ERR__OK;
}

snsr288x_error_t
snsr288x_get_temp_sensor_config(snsr288x_ctx_t* ctx,
                                snsr288x_temp_sensor_cfg_t* temp_cfg,
                                uint8_t* low_trip_cnt,
                                uint8_t* high_trip_cnt)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(NULL != temp_cfg);
  SNSR288X_ASSERT(NULL != low_trip_cnt);
  SNSR288X_ASSERT(NULL != high_trip_cnt);

  // Read all temp sensor configuration registers
  READ_REG_RANGE(
    &ctx->scratch[0], TEMPERATURE_CONFIGURATION, TEMP_ALARM_HIGH_LSB);
  uint8_t* const TEMPERATURE_CONFIGURATION = &ctx->scratch[0x0];
  uint8_t* const TEMP_ALARM_LOW_SETUP = &ctx->scratch[0x1];
  uint8_t* const TEMP_ALARM_HIGH_SETUP = &ctx->scratch[0x2];
  uint8_t* const TEMP_ALARM_LOW_MSB = &ctx->scratch[0x3];
  uint8_t* const TEMP_ALARM_LOW_LSB = &ctx->scratch[0x4];
  uint8_t* const TEMP_ALARM_HIGH_MSB = &ctx->scratch[0x5];
  uint8_t* const TEMP_ALARM_HIGH_LSB = &ctx->scratch[0x6];

  temp_cfg->enable = SNSR288X_GET_FIELD(
    TEMPERATURE_CONFIGURATION, TEMP_SELECT, *TEMPERATURE_CONFIGURATION);
  temp_cfg->alarm_low_cfg.differential =
    false; // Differential mode is not possible for this sensor
  temp_cfg->alarm_low_cfg.nonconsecutive = SNSR288X_GET_FIELD(
    TEMP_ALARM_LOW_SETUP, TEMP_LO_TRIP, *TEMP_ALARM_LOW_SETUP);
  temp_cfg->alarm_low_cfg.trip_count = SNSR288X_GET_FIELD(
    TEMP_ALARM_LOW_SETUP, TEMP_LO_TRIP_CNT, *TEMP_ALARM_LOW_SETUP);
  temp_cfg->alarm_low_cfg.threshold =
    *TEMP_ALARM_LOW_LSB + (((uint16_t)*TEMP_ALARM_LOW_MSB) << 8);
  *low_trip_cnt = SNSR288X_GET_FIELD(
    TEMP_ALARM_LOW_SETUP, TEMP_LO_DET_CNTR, *TEMP_ALARM_LOW_SETUP);
  temp_cfg->alarm_high_cfg.differential =
    false; // Differential mode is not possible for this sensor
  temp_cfg->alarm_high_cfg.nonconsecutive = SNSR288X_GET_FIELD(
    TEMP_ALARM_HIGH_SETUP, TEMP_HI_TRIP, *TEMP_ALARM_HIGH_SETUP);
  temp_cfg->alarm_high_cfg.trip_count = SNSR288X_GET_FIELD(
    TEMP_ALARM_HIGH_SETUP, TEMP_HI_TRIP_CNT, *TEMP_ALARM_HIGH_SETUP);
  temp_cfg->alarm_high_cfg.threshold =
    *TEMP_ALARM_HIGH_LSB + (((uint16_t)*TEMP_ALARM_HIGH_MSB) << 8);
  *high_trip_cnt = SNSR288X_GET_FIELD(
    TEMP_ALARM_HIGH_SETUP, TEMP_HI_DET_CNTR, *TEMP_ALARM_HIGH_SETUP);

  return SNSR288X_ERR__OK;
}

snsr288x_error_t
snsr288x_configure_temp_sensor(snsr288x_ctx_t* ctx,
                               const snsr288x_temp_sensor_cfg_t* temp_cfg,
                               bool reset)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(SNSR288X_TEMP_SENSOR_CFG_IS_VALID(temp_cfg));

  // Populate all temp sensor configuration registers
  uint8_t* const TEMPERATURE_CONFIGURATION = &ctx->scratch[0x0];
  uint8_t* const TEMP_ALARM_LOW_SETUP = &ctx->scratch[0x1];
  uint8_t* const TEMP_ALARM_HIGH_SETUP = &ctx->scratch[0x2];
  uint8_t* const TEMP_ALARM_LOW_MSB = &ctx->scratch[0x3];
  uint8_t* const TEMP_ALARM_LOW_LSB = &ctx->scratch[0x4];
  uint8_t* const TEMP_ALARM_HIGH_MSB = &ctx->scratch[0x5];
  uint8_t* const TEMP_ALARM_HIGH_LSB = &ctx->scratch[0x6];
  INIT_REG_RANGE(
    &ctx->scratch[0], TEMPERATURE_CONFIGURATION, TEMP_ALARM_HIGH_LSB);

  *TEMPERATURE_CONFIGURATION = SNSR288X_SET_FIELD(TEMPERATURE_CONFIGURATION,
                                                  TEMP_SELECT,
                                                  *TEMPERATURE_CONFIGURATION,
                                                  temp_cfg->enable);
  *TEMP_ALARM_LOW_SETUP =
    SNSR288X_SET_FIELD(TEMP_ALARM_LOW_SETUP,
                       TEMP_LO_TRIP,
                       *TEMP_ALARM_LOW_SETUP,
                       temp_cfg->alarm_low_cfg.nonconsecutive);
  *TEMP_ALARM_LOW_SETUP =
    SNSR288X_SET_FIELD(TEMP_ALARM_LOW_SETUP,
                       TEMP_LO_TRIP_CNT,
                       *TEMP_ALARM_LOW_SETUP,
                       temp_cfg->alarm_low_cfg.trip_count);
  *TEMP_ALARM_LOW_SETUP = SNSR288X_SET_FIELD(TEMP_ALARM_LOW_SETUP,
                                             TEMP_RST_LO_CNTR,
                                             *TEMP_ALARM_LOW_SETUP,
                                             reset); // Reset the counter
  *TEMP_ALARM_LOW_LSB = 0xFF & temp_cfg->alarm_low_cfg.threshold;
  *TEMP_ALARM_LOW_MSB = 0xFF & (temp_cfg->alarm_low_cfg.threshold >> 8);

  *TEMP_ALARM_HIGH_SETUP =
    SNSR288X_SET_FIELD(TEMP_ALARM_HIGH_SETUP,
                       TEMP_HI_TRIP,
                       *TEMP_ALARM_HIGH_SETUP,
                       temp_cfg->alarm_high_cfg.nonconsecutive);
  *TEMP_ALARM_HIGH_SETUP =
    SNSR288X_SET_FIELD(TEMP_ALARM_HIGH_SETUP,
                       TEMP_HI_TRIP_CNT,
                       *TEMP_ALARM_HIGH_SETUP,
                       temp_cfg->alarm_high_cfg.trip_count);
  *TEMP_ALARM_HIGH_SETUP = SNSR288X_SET_FIELD(TEMP_ALARM_HIGH_SETUP,
                                              TEMP_RST_HI_CNTR,
                                              *TEMP_ALARM_HIGH_SETUP,
                                              reset); // Reset the counter
  *TEMP_ALARM_HIGH_LSB = 0xFF & temp_cfg->alarm_high_cfg.threshold;
  *TEMP_ALARM_HIGH_MSB = 0xFF & (temp_cfg->alarm_high_cfg.threshold >> 8);

  // Write out the registers
  WRITE_REG_RANGE(
    &ctx->scratch[0], TEMPERATURE_CONFIGURATION, TEMP_ALARM_HIGH_LSB);

  return SNSR288X_ERR__OK;
}

snsr288x_error_t
snsr288x_get_dac_config(snsr288x_ctx_t* ctx,
                        snsr288x_dac_sel_t dac_sel,
                        snsr288x_dac_cfg_t* dac_cfg)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(SNSR288X_DAC_SEL_IS_VALID(dac_sel));
  SNSR288X_ASSERT(NULL != dac_cfg);

  // Read all DAC_N configuration registers
  spi_read(
    ctx, FIRST_REG_IN_DAC(dac_sel), &ctx->scratch[0], NUMBER_OF_DAC_REGS);
  uint8_t* const DAC_MSB_CODE = &ctx->scratch[0x0];
  uint8_t* const DAC_EN_LSB_CODE = &ctx->scratch[0x1];

  dac_cfg->dac_code =
    (SNSR288X_GET_FIELD(DACN_MSB_CODE, DACN_CODE, *DAC_MSB_CODE) << 8) +
    SNSR288X_GET_FIELD(DACN_EN_LSB_CODE, DACN_CODE, *DAC_EN_LSB_CODE);

  dac_cfg->enable =
    SNSR288X_GET_FIELD(DACN_EN_LSB_CODE, DACN_EN, *DAC_EN_LSB_CODE);

  return SNSR288X_ERR__OK;
}

snsr288x_error_t
snsr288x_configure_dac(snsr288x_ctx_t* ctx,
                       snsr288x_dac_sel_t dac_sel,
                       const snsr288x_dac_cfg_t* dac_cfg)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(SNSR288X_DAC_SEL_IS_VALID(dac_sel));
  SNSR288X_ASSERT(SNSR288X_DAC_CFG_IS_VALID(dac_cfg));

  // Populate all DAC_N configuration registers
  spi_read(
    ctx, FIRST_REG_IN_DAC(dac_sel), &ctx->scratch[0], NUMBER_OF_DAC_REGS);
  uint8_t* const DAC_MSB_CODE = &ctx->scratch[0x0];
  uint8_t* const DAC_EN_LSB_CODE = &ctx->scratch[0x1];

  *DAC_MSB_CODE = SNSR288X_SET_FIELD(
    DACN_MSB_CODE, DACN_CODE, *DAC_MSB_CODE, (dac_cfg->dac_code >> 8) & 0xFF);

  *DAC_EN_LSB_CODE = SNSR288X_SET_FIELD(
    DACN_EN_LSB_CODE, DACN_CODE, *DAC_EN_LSB_CODE, dac_cfg->dac_code & 0xFF);
  *DAC_EN_LSB_CODE = SNSR288X_SET_FIELD(
    DACN_EN_LSB_CODE, DACN_EN, *DAC_EN_LSB_CODE, dac_cfg->enable);

  // Write out the registers
  spi_write(
    ctx, FIRST_REG_IN_DAC(dac_sel), &ctx->scratch[0], NUMBER_OF_DAC_REGS);

  return SNSR288X_ERR__OK;
}

snsr288x_error_t
snsr288x_start_capture(snsr288x_ctx_t* ctx,
                       const snsr288x_capture_cfg_t* capture_cfg)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));
  SNSR288X_ASSERT(SNSR288X_CAPTURE_CFG_IS_VALID(capture_cfg));

  // Look up how many sensor channels this device has
  const uint8_t num_channels = NUM_CHANNELS_IN_PART(ctx->drv_cfg.part_id);

  // Read all relevant channel registers
  for (int chan = SNSR288X_CHAN__MIN; chan < num_channels; chan++) {
    const uint8_t cfg1_addr = FIRST_REG_IN_CHANNEL(chan);
    spi_read(ctx,
             cfg1_addr,
             &ctx->scratch[SNSR288X_CHANNEL_ADDR_RANGE_SIZE * chan],
             SNSR288X_CHANNEL_ADDR_RANGE_SIZE);
  }

  // Read all relevant EIS registers
  uint8_t eis_afe_1_val = spi_read_reg(ctx, SNSR288X_REG__EIS_AFE_1__ADDR);

  // Read the conversion config registers
  uint8_t conversion_regs[4] = { 0 }; // Beware skipped register 0x82
  uint8_t* const CONVERT_SETUP_1_VAL = &conversion_regs[0];
  uint8_t* const CONVERT_SETUP_2_VAL = &conversion_regs[1];
  // uint8_t* const RESERVED_VAL = &conversion_regs[2];
  uint8_t* const CONVERT_START_VAL = &conversion_regs[3];
  READ_REG_RANGE(conversion_regs, CONVERT_SETUP_1, CONVERT_START);

  // Stop any running conversions
  *CONVERT_START_VAL =
    SNSR288X_SET_FIELD(CONVERT_START, CONVERT, *CONVERT_START_VAL, false);
  spi_write_reg(ctx, SNSR288X_REG__CONVERT_START__ADDR, *CONVERT_START_VAL);

  // Disable all detectors and DC sensor amplifiers
  for (int chan = SNSR288X_CHAN__MIN; chan < num_channels; chan++) {
    const uint8_t offset = SNSR288X_CHANNEL_ADDR_RANGE_SIZE * chan;
    uint8_t* const CFG1_VAL = &ctx->scratch[offset + 0];
    // uint8_t* const CFG2_VAL = &ctx->scratch[offset + 1];
    uint8_t* const CFG3_VAL = &ctx->scratch[offset + 2];
    // uint8_t* const CFG4_VAL = &ctx->scratch[offset + 3];
    uint8_t* const CFG5_VAL = &ctx->scratch[offset + 4];

    // Disable amps and detectors, and deselect each channel
    *CFG1_VAL = SNSR288X_SET_FIELD(
      SENSOR_N_CONFIGURATION_1, SN_WE_AMP_EN, *CFG1_VAL, false);
    *CFG1_VAL = SNSR288X_SET_FIELD(
      SENSOR_N_CONFIGURATION_1, SN_CE_AMP_EN, *CFG1_VAL, false);
    *CFG3_VAL = SNSR288X_SET_FIELD(
      SENSOR_N_CONFIGURATION_3, SN_DETECTOR_EN, *CFG3_VAL, false);
    *CFG5_VAL =
      SNSR288X_SET_FIELD(SENSOR_N_CONFIGURATION_5, SN_SELECT, *CFG5_VAL, false);

    // Write back the registers
    spi_write(ctx,
              FIRST_REG_IN_CHANNEL(chan),
              &ctx->scratch[offset],
              SNSR288X_CHANNEL_ADDR_RANGE_SIZE);
  }

  // Disable EIS sensor amplifiers
  eis_afe_1_val =
    SNSR288X_SET_FIELD(EIS_AFE_1, EIS_WE_AMP_EN, eis_afe_1_val, false);
  eis_afe_1_val =
    SNSR288X_SET_FIELD(EIS_AFE_1, EIS_CE_AMP_EN, eis_afe_1_val, false);
  spi_write_reg(ctx, SNSR288X_REG__EIS_AFE_1__ADDR, eis_afe_1_val);

  // Enable/disable each sensor
  if (SNSR288X_SENSOR_MODE__DC_MEAS_OR_DETECTOR == capture_cfg->sensor_mode) {
    for (int chan = SNSR288X_CHAN__MIN; chan < num_channels; chan++) {
      const uint8_t offset = SNSR288X_CHANNEL_ADDR_RANGE_SIZE * chan;
      uint8_t* const CFG1_VAL = &ctx->scratch[offset + 0];
      // uint8_t* const CFG2_VAL = &ctx->scratch[offset + 1];
      uint8_t* const CFG3_VAL = &ctx->scratch[offset + 2];
      // uint8_t* const CFG4_VAL = &ctx->scratch[offset + 3];
      uint8_t* const CFG5_VAL = &ctx->scratch[offset + 4];

      // Should this channel be enabled?
      if (!capture_cfg->sensor.dc.enable[chan])
        continue;

      // Should this channel be configured as a detector, or for DC measurement?
      if (capture_cfg->sensor.dc.detector[chan]) {
        *CFG3_VAL = SNSR288X_SET_FIELD(
          SENSOR_N_CONFIGURATION_3, SN_DETECTOR_EN, *CFG3_VAL, true);
        spi_write_reg(ctx, FIRST_REG_IN_CHANNEL(chan) + 2, *CFG3_VAL);
      } else {
        *CFG1_VAL = SNSR288X_SET_FIELD(
          SENSOR_N_CONFIGURATION_1, SN_WE_AMP_EN, *CFG1_VAL, true);
        *CFG1_VAL = SNSR288X_SET_FIELD(
          SENSOR_N_CONFIGURATION_1, SN_CE_AMP_EN, *CFG1_VAL, true);
        *CFG5_VAL = SNSR288X_SET_FIELD(
          SENSOR_N_CONFIGURATION_5, SN_SELECT, *CFG5_VAL, true);

        // Write back the registers
        spi_write(ctx,
                  FIRST_REG_IN_CHANNEL(chan),
                  &ctx->scratch[offset],
                  SNSR288X_CHANNEL_ADDR_RANGE_SIZE);
      }
    }
  } else if (SNSR288X_SENSOR_MODE__EIS_MEASUREMENT ==
             capture_cfg->sensor_mode) {
    // TODO: EIS config
    return SNSR288X_ERR__UNSUPPORTED;
  }

  // Delay to let any changes settle, before starting conversions
  SNSR288X_DELAY_MS(ctx->drv_cfg.settle_time_ms);

  // Start the conversion
  *CONVERT_SETUP_1_VAL = SNSR288X_SET_FIELD(CONVERT_SETUP_1,
                                            SENS_PERIOD,
                                            *CONVERT_SETUP_1_VAL,
                                            capture_cfg->sensor.dc.period);
  *CONVERT_SETUP_1_VAL =
    SNSR288X_SET_FIELD(CONVERT_SETUP_1,
                       IOFFSET_CONV,
                       *CONVERT_SETUP_1_VAL,
                       capture_cfg->sensor.dc.offset_conversion);
  *CONVERT_SETUP_2_VAL = SNSR288X_SET_FIELD(CONVERT_SETUP_2,
                                            TEMP_PERIOD,
                                            *CONVERT_SETUP_2_VAL,
                                            capture_cfg->temp.period);
  *CONVERT_START_VAL = SNSR288X_SET_FIELD(
    CONVERT_START, AUTO, *CONVERT_START_VAL, capture_cfg->sensor.dc.auto_mode);
  *CONVERT_START_VAL =
    SNSR288X_SET_FIELD(CONVERT_START, CONVERT, *CONVERT_START_VAL, true);
  WRITE_REG_RANGE(conversion_regs, CONVERT_SETUP_1, CONVERT_START);

  return SNSR288X_ERR__OK;
}

snsr288x_error_t
snsr288x_stop_capture(snsr288x_ctx_t* ctx)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  // Look up how many sensor channels this device has
  const uint8_t num_channels = NUM_CHANNELS_IN_PART(ctx->drv_cfg.part_id);

  // Read all relevant channel registers
  for (int chan = SNSR288X_CHAN__MIN; chan < num_channels; chan++) {
    const uint8_t cfg1_addr = FIRST_REG_IN_CHANNEL(chan);
    spi_read(ctx,
             cfg1_addr,
             &ctx->scratch[SNSR288X_CHANNEL_ADDR_RANGE_SIZE * chan],
             SNSR288X_CHANNEL_ADDR_RANGE_SIZE);
  }

  // Read all relevant EIS registers
  uint8_t eis_afe_1_val = spi_read_reg(ctx, SNSR288X_REG__EIS_AFE_1__ADDR);

  // Read the converstion config registers
  uint8_t convert_start_val =
    spi_read_reg(ctx, SNSR288X_REG__CONVERT_START__ADDR);

  // Stop any running conversions
  convert_start_val =
    SNSR288X_SET_FIELD(CONVERT_START, CONVERT, convert_start_val, false);
  spi_write_reg(ctx, SNSR288X_REG__CONVERT_START__ADDR, convert_start_val);

  // Disable all detectors, DC sensors, and EIS, to prevent conflicting
  // configurations
  for (int chan = SNSR288X_CHAN__MIN; chan < num_channels; chan++) {
    const uint8_t offset = SNSR288X_CHANNEL_ADDR_RANGE_SIZE * chan;
    uint8_t* const CFG1_VAL = &ctx->scratch[offset + 0];
    // uint8_t* const CFG2_VAL = &ctx->scratch[offset + 1];
    uint8_t* const CFG3_VAL = &ctx->scratch[offset + 2];
    // uint8_t* const CFG4_VAL = &ctx->scratch[offset + 3];
    // uint8_t* const CFG5_VAL = &ctx->scratch[offset + 4];

    // Stop the conversion
    *CFG1_VAL = SNSR288X_SET_FIELD(
      SENSOR_N_CONFIGURATION_1, SN_WE_AMP_EN, *CFG1_VAL, false);
    *CFG1_VAL = SNSR288X_SET_FIELD(
      SENSOR_N_CONFIGURATION_1, SN_CE_AMP_EN, *CFG1_VAL, false);
    *CFG3_VAL = SNSR288X_SET_FIELD(
      SENSOR_N_CONFIGURATION_3, SN_DETECTOR_EN, *CFG3_VAL, false);

    // Write back the registers
    spi_write(ctx,
              FIRST_REG_IN_CHANNEL(chan),
              &ctx->scratch[offset],
              SNSR288X_CHANNEL_ADDR_RANGE_SIZE);
  }
  eis_afe_1_val =
    SNSR288X_SET_FIELD(EIS_AFE_1, EIS_WE_AMP_EN, eis_afe_1_val, false);
  eis_afe_1_val =
    SNSR288X_SET_FIELD(EIS_AFE_1, EIS_CE_AMP_EN, eis_afe_1_val, false);
  spi_write_reg(ctx, SNSR288X_REG__EIS_AFE_1__ADDR, eis_afe_1_val);

  return SNSR288X_ERR__OK;
}

/*******************************************************************************
 * Private Function Definitions
 ******************************************************************************/
static void
spi_cmd_transfer(snsr288x_ctx_t* ctx,
                 uint8_t reg,
                 bool read,
                 const uint8_t* wdata,
                 uint8_t* rdata,
                 uint16_t len)
{
  const uint8_t mosi[] = { reg, (read) ? SPI_CMD_READ : SPI_CMD_WRITE };
  uint8_t miso[sizeof(mosi)];

  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  SNSR288X_ASSERT(
    ctx->phy.spi_select(true) && ctx->phy.spi_transfer(mosi, miso, 2) &&
    ctx->phy.spi_transfer(wdata, rdata, len) && ctx->phy.spi_select(false));
}

static void
spi_read(snsr288x_ctx_t* ctx, uint8_t reg, uint8_t* data, uint16_t len)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  uint8_t bitbucket[len];
  spi_cmd_transfer(ctx, reg, true, bitbucket, data, len);
}

static void
spi_write(snsr288x_ctx_t* ctx, uint8_t reg, const uint8_t* data, uint16_t len)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  uint8_t bitbucket[len];
  spi_cmd_transfer(ctx, reg, false, data, bitbucket, len);
}

static uint8_t
spi_read_reg(snsr288x_ctx_t* ctx, uint8_t reg)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  uint8_t val;
  spi_read(ctx, reg, &val, 1);
  return val;
}

static void
spi_write_reg(snsr288x_ctx_t* ctx, uint8_t reg, uint8_t val)
{
  SNSR288X_ASSERT(SNSR288X_CTX_IS_VALID(ctx));

  spi_write(ctx, reg, &val, 1);
}
