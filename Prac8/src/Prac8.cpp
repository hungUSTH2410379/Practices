/*
 * Practice 8: I2C and sensor - Grove Beginner Kit for Arduino
 * ---------------------------------------------------------------
 * ONE FILE version: the low-level I2C driver and the BMP280 sensor
 * code are all combined here - no separate i2c.h / i2c.cpp needed.
 *
 * WIRING: nothing to wire up - the Air Pressure module's Grove cable
 * is already plugged into one of the shield's "I2C" sockets, which are
 * hard-wired to the ATmega328P's hardware TWI pins:
 *     SDA -> A4 (PC4)
 *     SCL -> A5 (PC5)
 */

#include <Arduino.h>
#include <avr/io.h>
#include <stdint.h>

/* =========================================================================
 *  SECTION 1: I2C (TWI) driver - register-level, no Wire.h
 * ========================================================================= */

#define I2C_WRITE 0   // SLA+W : bit 0 = 0
#define I2C_READ  1   // SLA+R : bit 0 = 1
#define I2C_SCL_FREQ 100000UL   // 100 kHz, I2C "Standard Mode"

// Configure TWI bit-rate for 100 kHz SCL, then enable the TWI hardware.
void i2c_init(void) {
    TWSR = 0x00;  // prescaler = 1 (TWPS1:0 = 00)
    TWBR = (uint8_t)(((F_CPU / I2C_SCL_FREQ) - 16) / 2);
    TWCR = (1 << TWEN);
}

// START condition (or repeated START if bus already active).
void i2c_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

// STOP condition.
void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO));
}

// Send one byte (address byte OR a data byte).
void i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

// Read one byte, master replies ACK ("send more").
uint8_t i2c_read_ack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

// Read one byte, master replies NACK ("that was the last byte").
uint8_t i2c_read_nack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

// Status code of the last TWI operation (top 5 bits of TWSR).
uint8_t i2c_get_status(void) {
    return (TWSR & 0xF8);
}

/* =========================================================================
 *  SECTION 2: BMP280 air-pressure / temperature sensor (I2C)
 * ========================================================================= */

#define BMP280_ADDR_PRIMARY    0x77   // Grove's default (right pads bridged)
#define BMP280_ADDR_SECONDARY  0x76   // fallback for some boards/clones
#define BMP280_CHIP_ID         0x58

#define REG_CHIP_ID     0xD0
#define REG_CTRL_MEAS   0xF4
#define REG_CONFIG      0xF5
#define REG_CALIB_START 0x88   // 24 bytes of calibration data start here
#define REG_DATA_START  0xF7   // press(3 bytes) + temp(3 bytes)

static uint8_t bmp280_addr = BMP280_ADDR_PRIMARY;

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
} bmp280_calib_t;

static bmp280_calib_t calib;
static float t_fine;   // shared between temp/pressure compensation, per datasheet

// Read `len` bytes starting at register `reg`: write the pointer, then a
// fresh START to burst-read it back (ACK on every byte except the last).
static void bmp280_read_regs(uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_start();
    i2c_write((bmp280_addr << 1) | I2C_WRITE);
    i2c_write(reg);
    i2c_stop();

    i2c_start();
    i2c_write((bmp280_addr << 1) | I2C_READ);
    for (uint8_t i = 0; i < len - 1; i++) buf[i] = i2c_read_ack();
    buf[len - 1] = i2c_read_nack();
    i2c_stop();
}

static void bmp280_write_reg(uint8_t reg, uint8_t value) {
    i2c_start();
    i2c_write((bmp280_addr << 1) | I2C_WRITE);
    i2c_write(reg);
    i2c_write(value);
    i2c_stop();
}

// Find the sensor (0x77 then 0x76), load its calibration constants,
// and start it in continuous (Normal) measurement mode.
bool bmp280_init(void) {
    uint8_t id;

    bmp280_addr = BMP280_ADDR_PRIMARY;
    bmp280_read_regs(REG_CHIP_ID, &id, 1);
    if (id != BMP280_CHIP_ID) {
        bmp280_addr = BMP280_ADDR_SECONDARY;
        bmp280_read_regs(REG_CHIP_ID, &id, 1);
        if (id != BMP280_CHIP_ID) return false;
    }

    uint8_t raw[24];
    bmp280_read_regs(REG_CALIB_START, raw, 24);
    calib.dig_T1 = (uint16_t)(raw[1] << 8 | raw[0]);
    calib.dig_T2 = (int16_t)(raw[3] << 8 | raw[2]);
    calib.dig_T3 = (int16_t)(raw[5] << 8 | raw[4]);
    calib.dig_P1 = (uint16_t)(raw[7] << 8 | raw[6]);
    calib.dig_P2 = (int16_t)(raw[9] << 8 | raw[8]);
    calib.dig_P3 = (int16_t)(raw[11] << 8 | raw[10]);
    calib.dig_P4 = (int16_t)(raw[13] << 8 | raw[12]);
    calib.dig_P5 = (int16_t)(raw[15] << 8 | raw[14]);
    calib.dig_P6 = (int16_t)(raw[17] << 8 | raw[16]);
    calib.dig_P7 = (int16_t)(raw[19] << 8 | raw[18]);
    calib.dig_P8 = (int16_t)(raw[21] << 8 | raw[20]);
    calib.dig_P9 = (int16_t)(raw[23] << 8 | raw[22]);

    bmp280_write_reg(REG_CTRL_MEAS, 0b00100111); // osrs_t=x1, osrs_p=x1, mode=Normal
    bmp280_write_reg(REG_CONFIG, 0x00);          // standby 0.5ms, filter off

    return true;
}

// Read raw registers and apply Bosch's compensation formulas.
void bmp280_read(float *temp_c, float *pressure_hpa) {
    uint8_t d[6];
    bmp280_read_regs(REG_DATA_START, d, 6);

    int32_t adc_P = ((int32_t)d[0] << 12) | ((int32_t)d[1] << 4) | (d[2] >> 4);
    int32_t adc_T = ((int32_t)d[3] << 12) | ((int32_t)d[4] << 4) | (d[5] >> 4);

    // temperature compensation
    float var1 = (((float)adc_T) / 16384.0f - ((float)calib.dig_T1) / 1024.0f) * ((float)calib.dig_T2);
    float var2 = ((((float)adc_T) / 131072.0f - ((float)calib.dig_T1) / 8192.0f) *
                  (((float)adc_T) / 131072.0f - ((float)calib.dig_T1) / 8192.0f)) * ((float)calib.dig_T3);
    t_fine = var1 + var2;
    *temp_c = (var1 + var2) / 5120.0f;

    // pressure compensation (uses t_fine from above)
    float p1 = ((float)t_fine / 2.0f) - 64000.0f;
    float p2 = p1 * p1 * ((float)calib.dig_P6) / 32768.0f;
    p2 = p2 + p1 * ((float)calib.dig_P5) * 2.0f;
    p2 = (p2 / 4.0f) + (((float)calib.dig_P4) * 65536.0f);
    p1 = (((float)calib.dig_P3) * p1 * p1 / 524288.0f + ((float)calib.dig_P2) * p1) / 524288.0f;
    p1 = (1.0f + p1 / 32768.0f) * ((float)calib.dig_P1);

    if (p1 == 0.0f) { *pressure_hpa = 0.0f; return; }

    float p = 1048576.0f - (float)adc_P;
    p = (p - (p2 / 4096.0f)) * 6250.0f / p1;
    p1 = ((float)calib.dig_P9) * p * p / 2147483648.0f;
    p2 = p * ((float)calib.dig_P8) / 32768.0f;
    p = p + (p1 + p2 + ((float)calib.dig_P7)) / 16.0f;

    *pressure_hpa = p / 100.0f;
}

/* =========================================================================
 *  SECTION 3: setup() / loop()
 * ========================================================================= */

void setup() {
    Serial.begin(115200);
    i2c_init();
    delay(300);

    if (bmp280_init()) {
        Serial.print(F("BMP280 found at 0x"));
        Serial.println(bmp280_addr, HEX);
    } else {
        Serial.println(F("BMP280 not found at 0x76 or 0x77 - check the Grove cable is in an I2C socket."));
    }
}

void loop() {
    float temp_c, pressure_hpa;
    bmp280_read(&temp_c, &pressure_hpa);

    Serial.print(F("Temp: "));
    Serial.print(temp_c, 1);
    Serial.print(F(" C   Pressure: "));
    Serial.print(pressure_hpa, 1);
    Serial.println(F(" hPa"));

    delay(1000);
}