#include "adc.h"

// Constructor: Inicializa el objeto ADC con un puntero I2C nulo
ADC::ADC() : i2c(nullptr) {}

// Inicializa el ADC con el puntero I2C dado
void ADC::init(I2C* i2c_pointer) {
    i2c = i2c_pointer;
}

// Lee el valor del ADC del canal especificado y lo almacena en el puntero de valor proporcionado
void ADC::read(uint8_t channel, int16_t* value) {
    // Valida el canal (0 a 3)
    if (channel > 3) return;

    // Configura MUX[14:12] para el canal seleccionado en modo de entrada única
    uint8_t mux = 0x04 + channel; // MUX[14:12] = 100 + canal

    // Configura el registro de configuración
    uint16_t config = 0;
    config |= (1 << 15);      // OS = 1 (Iniciar una conversión única)
    config |= (mux << 12);    // MUX[14:12]: Canal seleccionado con respecto a GND
    config |= (0 << 9);       // PGA[11:9] = 000 (±6.144V)
    config |= (1 << 8);       // MODE = 1 (Modo de conversión única)
    config |= (4 << 5);       // DR[7:5] = 100 (128 SPS)
    config |= (0x03);         // COMP_QUE[1:0] = 11 (Desactivar el comparador)

    // Escribe en el registro de configuración
    uint8_t data[3];
    data[0] = 0x01;                   // Dirección del registro de configuración
    data[1] = (config >> 8) & 0xFF;   // MSB
    data[2] = config & 0xFF;          // LSB
    i2c->write(ADS1115_ADDR, data, 3);

    // Espera a que la conversión se complete (según la tasa de datos seleccionada)
    delay(ADC_CONVERSION_TIME * 3); // 8ms para 128 SPS

    // Lee el registro de conversión
    data[0] = 0x00; // Dirección del registro de conversión
    i2c->write(ADS1115_ADDR, data, 1);
    i2c->read(ADS1115_ADDR, data, 2);

    // Combina el MSB y el LSB para obtener el valor de 16 bits
    int16_t raw_adc = (data[0] << 8) | data[1];

    // Asegura que el valor sea positivo
    if (raw_adc < 0) {
        raw_adc = 0;
    }

    // Devuelve el valor del ADC
    *value = raw_adc;
}

// Lee el valor del ADC del canal especificado y lo convierte a voltaje
float ADC::read_voltage(uint8_t channel) {
    int16_t v = 0;
    read(channel, &v);

    // Convierte el valor del ADC a voltaje
    return 2 * ADC_PGA * ((float)v) / ((float)ADC_MAX_VALUE); // V = 2 * PGA * ADC / 2^16
}