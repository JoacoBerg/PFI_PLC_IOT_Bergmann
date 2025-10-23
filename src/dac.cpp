#include "dac.h"

// Constructor: Inicializa el objeto DAC con un puntero I2C nulo
DAC::DAC() : i2c(nullptr) {}

// Inicializa el DAC con el puntero I2C dado y establece el voltaje a 0V
void DAC::init(I2C* i2c_pointer){
    i2c = i2c_pointer;
    set_voltage(0); // Establece el DAC al valor predeterminado (0V)
}

// Establece el voltaje de salida del DAC en milivoltios
void DAC::set_voltage(int voltageInMmV) {
    // Calcula el valor del DAC
    float value = ((voltageInMmV / 1000.0) / DAC_V_MAX) * DAC_RESOLUTION; // Convierte mV a V y escala a la resolución del DAC
    // Redondea al entero más cercano
    digitalWrite((uint16_t)(value + 0.5));
}

// Escribe el valor dado en el DAC usando la interfaz I2C
void DAC::digitalWrite(uint16_t value) {
    if (value > DAC_MAX_DIGITAL_VALUE) { // Verifica si el valor está fuera de rango
        Serial.println("Valor del DAC fuera de rango");
        return;
    }

    // Escribe el valor en el DAC usando la biblioteca I2C
    uint8_t data[2];
    data[0] = (value >> 8) & 0x0F;
    data[1] = value & 0xFF;
    i2c->write(MCP4725_ADDR, data, 2);
}

// Lee el voltaje de salida actual del DAC en milivoltios
int DAC::readVoltage() {
    if (!i2c) { // Verifica si el puntero I2C es válido
        Serial.println("I2C no inicializado");
        return -1; // Error
    }

    uint8_t data[3];
    i2c->read(MCP4725_ADDR, data, 3);    

    // Decodifica los datos
    uint16_t dac_value = ((data[0] & 0x0F) << 8) | data[1]; // Bits 11-0
    dac_value = (dac_value << 4) | ((data[2] & 0xF0) >> 4); // Bits 3-0 de data[2]

    // Convierte a voltaje
    float voltage = (float)dac_value / (DAC_RESOLUTION - 1) * DAC_V_MAX;

    return (int)(voltage * 1000.0 + 0.5); // Devuelve el voltaje en mV
}