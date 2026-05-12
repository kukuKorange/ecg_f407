/**
  ******************************************************************************
  * @file    ecg_sim.c
  * @brief   ECG信号模拟器 (与平台无关，直接移植)
  ******************************************************************************
  */

#include "ecg_sim.h"

#ifdef USE_ECG_SIM

#define ECG_SIM_SAMPLE_RATE  200
#define ECG_SIM_BASELINE     2048

#define PM_P_START      80
#define PM_P_END        200
#define PM_Q_START      245
#define PM_Q_END        275
#define PM_R_RISE_START 275
#define PM_R_RISE_END   315
#define PM_R_FALL_END   380
#define PM_S_END        460
#define PM_T_START      510
#define PM_T_END        810

#define AMP_P    250
#define AMP_Q    200
#define AMP_R   1500
#define AMP_S    400
#define AMP_T    500

static uint8_t  sim_bpm       = ECG_SIM_BPM;
static uint16_t sim_cycle_len = 0;
static uint16_t sim_phase     = 0;

#define SIM_BPM_MIN   60
#define SIM_BPM_MAX   90

static int8_t  sim_wander_dir  =  1;
static uint8_t sim_wander_hold =  0;
static uint8_t sim_wander_next = 10;
static uint8_t sim_lfsr        = 0xA5u;

static void prv_wander_step(void)
{
    sim_wander_hold++;
    if (sim_wander_hold < sim_wander_next) return;
    sim_wander_hold = 0;

    if (sim_wander_dir > 0)
    {
        if (sim_bpm < SIM_BPM_MAX) sim_bpm++;
        else sim_wander_dir = -1;
    }
    else
    {
        if (sim_bpm > SIM_BPM_MIN) sim_bpm--;
        else sim_wander_dir = 1;
    }

    sim_cycle_len = (uint16_t)((uint32_t)ECG_SIM_SAMPLE_RATE * 60u / sim_bpm);
    sim_lfsr = (uint8_t)((sim_lfsr >> 1) ^ ((uint8_t)(-(sim_lfsr & 1u)) & 0xB8u));
    sim_wander_next = 7 + (sim_lfsr % 12u);
}

static int16_t ecg_sim_offset(uint16_t pm)
{
    int32_t t, T, val;

    if (pm >= PM_P_START && pm < PM_P_END)
    {
        t = pm - PM_P_START; T = PM_P_END - PM_P_START;
        val = (int32_t)4 * t * (T - t) * AMP_P / (T * T);
        return (int16_t)val;
    }
    if (pm >= PM_Q_START && pm < PM_Q_END)
    {
        t = pm - PM_Q_START; T = PM_Q_END - PM_Q_START;
        val = (int32_t)4 * t * (T - t) * AMP_Q / (T * T);
        return -(int16_t)val;
    }
    if (pm >= PM_R_RISE_START && pm < PM_R_RISE_END)
    {
        t = pm - PM_R_RISE_START; T = PM_R_RISE_END - PM_R_RISE_START;
        return (int16_t)((int32_t)t * AMP_R / T);
    }
    if (pm >= PM_R_RISE_END && pm < PM_R_FALL_END)
    {
        t = pm - PM_R_RISE_END; T = PM_R_FALL_END - PM_R_RISE_END;
        return (int16_t)(AMP_R - (int32_t)t * (AMP_R + AMP_S) / T);
    }
    if (pm >= PM_R_FALL_END && pm < PM_S_END)
    {
        t = pm - PM_R_FALL_END; T = PM_S_END - PM_R_FALL_END;
        return (int16_t)(-AMP_S + (int32_t)t * AMP_S / T);
    }
    if (pm >= PM_T_START && pm < PM_T_END)
    {
        t = pm - PM_T_START; T = PM_T_END - PM_T_START;
        val = (int32_t)4 * t * (T - t) * AMP_T / (T * T);
        return (int16_t)val;
    }
    return 0;
}

void ECG_Sim_Init(uint8_t bpm)
{
    if (bpm < SIM_BPM_MIN) bpm = SIM_BPM_MIN;
    if (bpm > SIM_BPM_MAX) bpm = SIM_BPM_MAX;
    sim_bpm       = bpm;
    sim_cycle_len = (uint16_t)((uint32_t)ECG_SIM_SAMPLE_RATE * 60u / bpm);
    sim_phase     = 0;
    sim_wander_dir  = 1;
    sim_wander_hold = 0;
    sim_wander_next = 10;
    sim_lfsr        = 0xA5u;
}

uint16_t ECG_Sim_GetSample(void)
{
    int32_t adc;
    uint16_t pm;

    pm = (uint16_t)((uint32_t)sim_phase * 1000u / sim_cycle_len);
    adc = (int32_t)ECG_SIM_BASELINE + ecg_sim_offset(pm);
    if (adc < 0)    adc = 0;
    if (adc > 4095) adc = 4095;

    sim_phase++;
    if (sim_phase >= sim_cycle_len)
    {
        sim_phase = 0;
        prv_wander_step();
    }
    return (uint16_t)adc;
}

void ECG_Sim_SetBPM(uint8_t bpm) { ECG_Sim_Init(bpm); }
uint8_t ECG_Sim_GetBPM(void) { return sim_bpm; }

#endif /* USE_ECG_SIM */
