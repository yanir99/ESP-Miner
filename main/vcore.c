#include <stdio.h>
#include <math.h>
#include "esp_log.h"

#include "vcore.h"
#include "adc.h"
#include "DS4432U.h"
#include "TPS546.h"

// DS4432U Transfer function constants for Bitaxe board
#define BITAXE_VFB 0.6
#define BITAXE_IFS 0.000098921 // (Vrfs / Rfs) x (127/16)  -> Vrfs = 0.997, Rfs = 80000
#define BITAXE_RA 4750.0       // R14
#define BITAXE_RB 3320.0       // R15
#define BITAXE_VNOM 1.451   // this is with the current DAC set to 0. Should be pretty close to (VFB*(RA+RB))/RB
#define BITAXE_VMAX 2.39
#define BITAXE_VMIN 0.046

static const char *TAG = "vcore.c";

void VCORE_init(GlobalState global_state) {
    if (global_state.device_model == DEVICE_HEX) {
        TPS546_init();
    }
    ADC_init();
}

/**
 * @brief ds4432_tps40305_bitaxe_voltage_to_reg takes a voltage and returns a register setting for the DS4432U to get that voltage on the TPS40305
 * careful with this one!!
 */
static uint8_t ds4432_tps40305_bitaxe_voltage_to_reg(float vout)
{
    float change;
    uint8_t reg;

    // make sure the requested voltage is in within range of BITAXE_VMIN and BITAXE_VMAX
    if (vout > BITAXE_VMAX || vout < BITAXE_VMIN)
    {
        return 0;
    }

    // this is the transfer function. comes from the DS4432U+ datasheet
    change = fabs((((BITAXE_VFB / BITAXE_RB) - ((vout - BITAXE_VFB) / BITAXE_RA)) / BITAXE_IFS) * 127);
    reg = (uint8_t)ceil(change);

    // Set the MSB high if the requested voltage is BELOW nominal
    if (vout < BITAXE_VNOM)
    {
        reg |= 0x80;
    }

    return reg;
}

bool VCORE_set_voltage(float core_voltage, GlobalState * global_state)
{
    uint8_t reg_setting;

    switch (global_state->device_model) {
        case DEVICE_MAX:
        case DEVICE_ULTRA:
        case DEVICE_SUPRA:
            reg_setting = ds4432_tps40305_bitaxe_voltage_to_reg(core_voltage * global_state->voltage_domain);
            ESP_LOGI(TAG, "Set ASIC voltage = %.3fV [0x%02X]", core_voltage, reg_setting);
            DS4432U_set_current_code(0, reg_setting); /// eek!
            break;
        // case DEVICE_HEX:
        default:
    }

    return true;
}

uint16_t VCORE_get_voltage_mv(GlobalState * global_state) {
    return ADC_get_vcore() / global_state->voltage_domain;
}
