#include "pwm.h"

// Constructor para inicializar el PWM con un pin específico, frecuencia y ciclo de trabajo
PWM::PWM(uint8_t pin, uint32_t frequency, uint8_t dutyCycle) 
    : _pin(pin), _frequency(frequency), _dutyCycle(dutyCycle) {
    pinMode(_pin, OUTPUT);
    analogWriteFrequency(_frequency);
    analogWrite(_pin, _dutyCycle);
}

// Método para establecer la frecuencia del PWM
void PWM::setFrequency(uint32_t frequency) {
    _frequency = frequency;
    analogWriteFrequency(_frequency);
}

// Método para establecer el ciclo de trabajo del PWM
void PWM::setDutyCycle(uint8_t dutyCycle) {
    // Ajustar entrada 0-100 a 0-255
    _dutyCycle = map(dutyCycle, 0, 100, 0, 255);
    analogWrite(_pin, _dutyCycle);
}

// Método para obtener la frecuencia actual del PWM
uint32_t PWM::getFrequency() const {
    return _frequency;
}

// Método para obtener el ciclo de trabajo actual del PWM
uint8_t PWM::getDutyCycle() const {
    return _dutyCycle;
}