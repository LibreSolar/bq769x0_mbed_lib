/*
    bq769x0.h - Battery management system based on bq769x0 for ARM mbed
    Copyright (C) 2015-2016  Martin Jäger (http://libre.solar)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this program. If not, see
    <http://www.gnu.org/licenses/>.
*/

#ifndef BQ769X0_H
#define BQ769X0_H

#include "mbed.h"

#define MAX_NUMBER_OF_CELLS 15
#define MAX_NUMBER_OF_THERMISTORS 3
#define NUM_OCV_POINTS 21

// IC type/size
#define bq76920 1
#define bq76930 2
#define bq76940 3

// output information to serial console for debugging
#define BQ769X0_DEBUG 1

class bq769x0 {

public:

    // initialization, status update and shutdown
    bq769x0(I2C& bqI2C, PinName alertPin, int bqType = bq76930, int bqI2CAddress = 0x08, bool crc = true);
    int checkStatus();  // returns 0 if everything is OK
    void update(void);
    void boot(PinName bootPin);
    void shutdown(void);

    // charging control
    bool enableCharging(void);
    void disableCharging(void);
    bool enableDischarging(void);
    void disableDischarging(void);

    // hardware settings
    void setShuntResistorValue(float res_mOhm);
    void setThermistorBetaValue(int beta_K);

    void resetSOC(int percent = -1);    // 0-100 %, -1 for automatic reset based on OCV
    void setBatteryCapacity(long capacity_mAh);
    void setOCV(int voltageVsSOC[NUM_OCV_POINTS]);

    // limit settings (for battery protection)
    void setTemperatureLimits(int minDischarge_degC, int maxDischarge_degC, int minCharge_degC, int maxCharge_degC);    // °C
    long setShortCircuitProtection(long current_mA, int delay_us = 70);
    long setOvercurrentChargeProtection(long current_mA, int delay_ms = 8);
    long setOvercurrentDischargeProtection(long current_mA, int delay_ms = 8);
    int setCellUndervoltageProtection(int voltage_mV, int delay_s = 1);
    int setCellOvervoltageProtection(int voltage_mV, int delay_s = 1);

    // balancing settings
    void setBalancingThresholds(int idleTime_min = 30, int absVoltage_mV = 3400, int voltageDifference_mV = 20);
    void setIdleCurrentThreshold(int current_mA);

    // automatic balancing when battery is within balancing thresholds
    void enableAutoBalancing(void);
    void disableAutoBalancing(void);

    // battery status
    int  getBatteryCurrent(void);
    int  getBatteryVoltage(void);
    int  getCellVoltage(int idCell);    // from 1 to 15
    int  getMinCellVoltage(void);
    int  getMaxCellVoltage(void);
    float getTemperatureDegC(int channel = 1);
    float getTemperatureDegF(int channel = 1);
    float getSOC(void);
    int getBalancingStatus(void);

    // interrupt handling (not to be called manually!)
    void setAlertInterruptFlag(void);

    #if BQ769X0_DEBUG
    void printRegisters(void);
    #endif

private:

    // Variables

    I2C& _i2c;
    //Serial _debug;
    Timer _timer;
    InterruptIn _alertInterrupt;

    int I2CAddress;
    int type;
    bool crcEnabled;

    float shuntResistorValue_mOhm;
    int thermistorBetaValue;  // typical value for Semitec 103AT-5 thermistor: 3435
    int *OCV;  // Open Circuit Voltage of cell for SOC 100%, 95%, ..., 5%, 0%

    // indicates if a new current reading or an error is available from BMS IC
    bool alertInterruptFlag;

    int numberOfCells;
    int cellVoltages[MAX_NUMBER_OF_CELLS];          // mV
    int idCellMaxVoltage;
    int idCellMinVoltage;
    long batVoltage;                                // mV
    long batCurrent;                                // mA
    int temperatures[MAX_NUMBER_OF_THERMISTORS];    // °C/10

    long nominalCapacity;    // mAs, nominal capacity of battery pack, max. 1193 Ah possible
    long coulombCounter;     // mAs (= milli Coulombs) for current integration

    // Current limits (mA)
    long maxChargeCurrent;
    long maxDischargeCurrent;
    int idleCurrentThreshold; // mA

    // Temperature limits (°C/10)
    int minCellTempCharge;
    int minCellTempDischarge;
    int maxCellTempCharge;
    int maxCellTempDischarge;

    // Cell voltage limits (mV)
    int maxCellVoltage;
    int minCellVoltage;
    int balancingMinCellVoltage_mV;
    int balancingMaxVoltageDifference_mV;

    int adcGain;    // uV/LSB
    int adcOffset;  // mV

    int errorStatus;
    bool autoBalancingEnabled;
    unsigned int balancingStatus;     // holds on/off status of balancing switches
    int balancingMinIdleTime_s;
    unsigned long idleTimestamp;

    unsigned int secSinceErrorCounter;
    unsigned long interruptTimestamp;

    // Methods

    void  updateVoltages(void);
    void  updateCurrent(void);
    void  updateTemperatures(void);

    void updateBalancingSwitches(void);

    int  readRegister(int address);
    void writeRegister(int address, int data);

};

#endif // BQ769X0_H
