#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    void GPIO_SetPinAsInput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
    void GPIO_SetPinAsOutput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
    void GPIO_SetPinAsAF(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
#ifdef __cplusplus
}
#endif
