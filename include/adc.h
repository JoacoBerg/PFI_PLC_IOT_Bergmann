/**
 * @file adc.h
 * @brief Archivo de encabezado para la clase ADC.
 * 
 * Este archivo contiene la declaración de la clase ADC, que proporciona métodos
 * para interactuar con un módulo Convertidor Analógico a Digital (ADC)
 * 
 * @note Esta clase requiere una instancia de I2C para la comunicación.
 * 
 */
#pragma once

#include <Wire.h>
#include <Arduino.h>
#include <i2c.h>

#define ADS1115_ADDR 0x48
#define ADC_CONVERSION_TIME 8
#define ADC_CHANNEL_IDUT 0
#define ADC_CHANNEL_TEMP 1
#define ADC_CHANNEL_VDUT 2
#define ADC_MAX_VALUE (1 << 16)
#define ADC_PGA 6.144

/**
 * @class ADC
 * @brief Una clase para interactuar con un módulo Convertidor Analógico a Digital (ADC).
 * 
 * La clase ADC proporciona métodos para inicializar el módulo ADC.
 * 
 * @note Esta clase requiere una instancia de I2C para la comunicación.
 */
class ADC {
public:
    /**
     * @brief Constructor para la clase ADC.
     * 
     * Inicializa el módulo Convertidor Analógico a Digital (ADC).
     */
    ADC();

    /**
     * @brief Inicializa el ADC con el puntero I2C dado.
     * 
     * Esta función configura el Convertidor Analógico a Digital (ADC) utilizando el puntero
     * I2C proporcionado. Configura los ajustes necesarios para habilitar la comunicación
     * con el ADC a través del bus I2C.
     * 
     * @param i2c_pointer Un puntero a una instancia de I2C utilizada para la comunicación con el ADC.
     */
    void init(I2C* i2c_pointer);

    /**
     * @brief Lee el voltaje del canal ADC especificado.
     * 
     * Esta función lee el voltaje analógico del canal ADC dado y 
     * devuelve el valor de voltaje correspondiente como un flotante.
     * 
     * @param channel El canal ADC del cual leer (0-255).
     * @return float El voltaje leído del canal ADC especificado.
     */
    float read_voltage(uint8_t channel);

private:
    /**
     * @brief Puntero a una instancia de I2C utilizada para la comunicación.
     */
    I2C* i2c;

    /**
     * @brief Lee el valor del ADC del canal especificado.
     * 
     * Esta función lee el valor del convertidor analógico a digital (ADC) del 
     * canal especificado y almacena el resultado en el puntero de valor proporcionado.
     * 
     * @param channel El canal ADC del cual leer.
     * @param value Puntero a una variable int16_t donde se almacenará el valor leído.
     */
    void read(uint8_t channel, int16_t* value);
};