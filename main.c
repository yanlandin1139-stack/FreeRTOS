#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

SemaphoreHandle_t xBinarySemaphore1 = NULL; // sinaliza Task2
SemaphoreHandle_t xBinarySemaphore2 = NULL; // sinaliza Task3
QueueHandle_t xQueue = NULL;                // token para Task1

static const char *aluno = "Yan Nolasco";

void Task1(void *pvParameters)
{
    int token;
    while (1)
    {
        // Espera o "token" vindo da Task3
        if (xQueueReceive(xQueue, &token, portMAX_DELAY) == pdTRUE)
        {
            printf("[Tarefa 1] Executou - %s\n", aluno);
            vTaskDelay(pdMS_TO_TICKS(1000));
            // Libera a Task2
            xSemaphoreGive(xBinarySemaphore1);
        }
    }
}

void Task2(void *pvParameters)
{
    while (1)
    {
        // Espera ser liberada pela Task1
        if (xSemaphoreTake(xBinarySemaphore1, portMAX_DELAY) == pdTRUE)
        {
            printf("[Tarefa 2] Executou - %s\n", aluno);
            vTaskDelay(pdMS_TO_TICKS(1000));
            // Libera a Task3
            xSemaphoreGive(xBinarySemaphore2);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void Task3(void *pvParameters)
{
    while (1)
    {
        // Espera ser liberada pela Task2
        if (xSemaphoreTake(xBinarySemaphore2, portMAX_DELAY) == pdTRUE)
        {
            printf("[Tarefa 3] Executou - %s\n", aluno);
            vTaskDelay(pdMS_TO_TICKS(1000));
            // Devolve o "token" para acordar a Task1 novamente
            int token = 1;
            xQueueSend(xQueue, &token, portMAX_DELAY);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void app_main(void)
{
    xBinarySemaphore1 = xSemaphoreCreateBinary();
    xBinarySemaphore2 = xSemaphoreCreateBinary();
    xQueue = xQueueCreate(1, sizeof(int));

    if (xBinarySemaphore1 == NULL || xBinarySemaphore2 == NULL || xQueue == NULL)
    {
        printf("Falha ao criar semáforos/fila\n");
        return;
    }
    
    int startToken = 1;
    xQueueSend(xQueue, &startToken, 0);

    xTaskCreate(Task1, "Task1", 2048, NULL, 5, NULL);
    xTaskCreate(Task2, "Task2", 2048, NULL, 5, NULL);
    xTaskCreate(Task3, "Task3", 2048, NULL, 5, NULL);
}