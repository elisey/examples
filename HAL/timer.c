#include "timer.h"
#include "stm32f10x.h"
#include <assert.h>

void Timer_Init()
{
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );

    TIM_TimeBaseInitTypeDef timerStruct;
    timerStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    timerStruct.TIM_CounterMode = TIM_CounterMode_Up;
    timerStruct.TIM_Period = 1200;
    timerStruct.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInit( TIM2, &timerStruct );

    TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Single);

    //TIM_ARRPreloadConfig(TIM2, ENABLE);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    //Сбрасываем флаги прерывания перед разрешением прерываний
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 14);
    NVIC_EnableIRQ(TIM2_IRQn);
    //TIM_Cmd(TIM2, ENABLE);
    DBGMCU->CR |= DBGMCU_CR_DBG_TIM2_STOP;
}

void Timer_SetTimer(uint16_t time)
{
    TIM_SetAutoreload(TIM2, time);
    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler()
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update))
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        Timer_onTimerOverflow();
    }
}

__weak void Timer_onTimerOverflow()
{
    assert(0);
}
