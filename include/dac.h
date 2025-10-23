/**
 * @file dac.h
 * @brief Archivo de encabezado para la clase DAC.
 * 
 * Este archivo contiene la definición de la clase DAC, que proporciona métodos
 * para inicializar y controlar un Convertidor Digital a Analógico (DAC) a través de un bus I2C.
 * Incluye funciones para establecer el voltaje de salida y escribir valores digitales en el DAC.
 * 
 * La clase DAC requiere una instancia de I2C para la comunicación.
 * 
 * @note La resolución del DAC es de 12 bits, con un voltaje de referencia de 5V.
 * 
 */
#pragma once

#include <Wire.h>
#include <Arduino.h>

#include "i2c.h"

#define MCP4725_ADDR 0x61   /*!< Dirección I2C del MCP4725 */

#define DAC_BITS 12 /*!< Resolución del DAC en bits */
#define DAC_RESOLUTION (1 << DAC_BITS) /*!< Resolución del DAC en bits */
#define DAC_REF_VOLTAGE 5   /*!< Voltaje de referencia en voltios */
#define DAC_Q (DAC_REF_VOLTAGE / DAC_RESOLUTION)    /*!< Paso de voltaje en V */
#define DAC_QmV (DAC_Q * 1000)  /*!< Paso de voltaje en mV */
#define DAC_V_MAX 5   /*!< Voltaje máximo de salida en V */
#define DAC_OUTPUT_VOLTAGE_DIVIDER (DAC_V_MAX / DAC_REF_VOLTAGE)    /*!< Divisor de voltaje de salida */
#define DAC_MAX_DIGITAL_VALUE (DAC_RESOLUTION - 1) /*!< Valor digital máximo */

/**
 * @class DAC
 * @brief Una clase para representar un Convertidor Digital a Analógico (DAC).
 * 
 * La clase DAC proporciona métodos para inicializar y controlar un dispositivo DAC
 * a través de un bus I2C. Permite establecer el voltaje de salida y escribir valores digitales
 * en el DAC.
 * 
 * @note Esta clase requiere una instancia de I2C para la comunicación.
 */
class DAC {
public:
    /**
     * @brief Construye un nuevo objeto DAC.
     * 
     * Este constructor inicializa una nueva instancia de la clase DAC.
     */
    DAC();

    /**
     * @brief Inicializa el DAC con el puntero I2C dado.
     * 
     * Esta función configura el DAC (Convertidor Digital a Analógico) utilizando el puntero
     * I2C proporcionado. Configura los ajustes necesarios para habilitar la comunicación
     * con el DAC a través del bus I2C.
     * 
     * @param i2c_pointer Un puntero a una instancia de I2C utilizada para la comunicación con el DAC.
     */
    void init(I2C* i2c_pointer);

    /**
     * @brief Establece el voltaje de salida del DAC.
     * 
     * Esta función establece el voltaje de salida del DAC (Convertidor Digital a Analógico)
     * al valor especificado en milivoltios (mV).
     * 
     * @param voltageInMmV El voltaje de salida deseado en milivoltios (mV).
     */
    void set_voltage(int voltageInMmV);

    /**
     * @brief Escribe un valor digital en el DAC.
     * 
     * Esta función establece la salida del DAC al valor digital especificado.
     * 
     * @param value El valor digital a escribir en el DAC. Este debe ser un 
     *              entero sin signo de 16 bits que representa el nivel de salida deseado.
     */
    void digitalWrite(uint16_t value);

    /**
     * @brief Lee el voltaje actual del DAC.
     * 
     * Esta función lee el nivel de voltaje actual del Convertidor Digital a Analógico (DAC)
     * y lo devuelve como un valor entero. La unidad específica y el rango del valor devuelto
     * dependen de la configuración y resolución del DAC.
     * 
     * @return int El nivel de voltaje actual leído del DAC.
     */
    int readVoltage();

private:
    I2C* i2c;
};