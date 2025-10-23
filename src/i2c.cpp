#include "i2c.h"

// Constructor de la clase I2C
I2C::I2C() {}

// Inicializar el bus I2C
void I2C::init() {
    Wire.begin(I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    Wire.setClock(I2C_MASTER_FREQ_HZ);
}

// Escribir datos a un dispositivo I2C
void I2C::write(uint8_t addr, uint8_t *data, size_t size) {
    Wire.beginTransmission(addr);
    for (size_t i = 0; i < size; i++) { // Escribir los datos al dispositivo I2C
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

// Leer datos de un dispositivo I2C
void I2C::read(uint8_t addr, uint8_t *data, size_t size) {
    // Solicitar datos del dispositivo I2C
    Wire.requestFrom(addr, size);
    for (size_t i = 0; i < size; i++) { // Leer los datos en el buffer
        if (Wire.available()) {
            data[i] = Wire.read();
        }
    }
}