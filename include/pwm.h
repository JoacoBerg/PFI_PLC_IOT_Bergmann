/**
 * @file pwm.h
 * @brief Archivo de encabezado para la clase PWM.
 *
 * Este archivo contiene la declaración de la clase PWM, que proporciona una interfaz
 * para controlar señales PWM (Modulación por Ancho de Pulso) en un pin específico.
 *
 */
#pragma once
#include <Arduino.h>

/**
 * @class PWM
 * @brief Una clase para controlar señales PWM (Modulación por Ancho de Pulso).
 * 
 * Esta clase proporciona una interfaz para configurar y controlar señales PWM en un pin especificado.
 * Permite establecer la frecuencia y el ciclo de trabajo de la señal PWM, así como recuperar
 * la configuración actual.
 * 
 */
class PWM {
public:

    /**
     * @brief Construye un nuevo objeto PWM.
     * 
     * @param pin El número de pin al que se enviará la señal PWM.
     * @param frequency La frecuencia de la señal PWM en Hertz.
     * @param dutyCycle El ciclo de trabajo de la señal PWM como un porcentaje (0-100).
     */
    PWM(uint8_t pin, uint32_t frequency, uint8_t dutyCycle);

    /**
     * @brief Establece la frecuencia para la señal PWM.
     * 
     * Esta función configura la señal PWM para operar a la frecuencia especificada.
     * 
     * @param frequency La frecuencia deseada en Hertz (Hz).
     */
    void setFrequency(uint32_t frequency);

    /**
     * @brief Establece el ciclo de trabajo para la señal PWM.
     * 
     * Esta función configura el ciclo de trabajo de la señal PWM (Modulación por Ancho de Pulso).
     * El ciclo de trabajo determina la proporción de tiempo que la señal está alta (activa) 
     * en comparación con el período total de la señal.
     * 
     * @param dutyCycle El ciclo de trabajo deseado como un porcentaje (0-100). 
     *                  Un valor de 0 significa que la señal está siempre baja,
     *                  y un valor de 100 significa que la señal está siempre alta.
     */
    void setDutyCycle(uint8_t dutyCycle);

    /**
     * @brief Recupera la frecuencia actual de la señal PWM.
     * 
     * @return uint32_t La frecuencia de la señal PWM en Hertz.
     */
    uint32_t getFrequency() const;

    /**
     * @brief Recupera el ciclo de trabajo actual de la señal PWM.
     * 
     * @return uint8_t El ciclo de trabajo actual como un porcentaje (0-100).
     */
    uint8_t getDutyCycle() const;
private:
    uint8_t _pin;
    uint32_t _frequency;
    uint8_t _dutyCycle;
};