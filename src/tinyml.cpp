#include "tinyml.h"
#include "global.h"
#include "DC_motor.h"
#include "relay_control.h"
#include "4_led_rgb.h"
#include "task_webserver.h"

extern void relay_set(bool on);

// Globals, for the convenience of one-shot setup.
namespace
{
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 8 * 1024; // Adjust size based on your model
    uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void setupTinyML()
{
    SERIAL_PRINTLN("TensorFlow Lite Init....");
    SERIAL_PRINTLN("Initializing Hardware...");
    dc_motor_setup();
    relay_setup();
    
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(dht_anomaly_model_tflite); // g_model_data is from model_data.h 
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        error_reporter->Report("Model provided is schema version %d, not equal to supported version %d.",
                               model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    SERIAL_PRINTLN("TensorFlow Lite Micro initialized on ESP32.");
}

void tiny_ml_task(void *pvParameters)
{

    setupTinyML();
    unsigned long last_ai_run = 0;
    const unsigned long AI_INTERVAL = 2000; 

    int current_label = -1; 

    while (1)
    {
        if (millis() - last_ai_run > AI_INTERVAL) 
        {
            last_ai_run = millis();

            // Read sensor data
            xSemaphoreTake(xSensorDataMutex, portMAX_DELAY);
            float temp = sharedSensorData.temperature;
            float humi = sharedSensorData.humidity;
            xSemaphoreGive(xSensorDataMutex);

            // Load input data
            input->data.f[0] = temp;
            input->data.f[1] = humi;

            // Run inference
            if (interpreter->Invoke() != kTfLiteOk) {
                error_reporter->Report("Invoke failed");
                continue;
            }

            // Softmax model returns 3 probabilities
            float p0 = output->data.f[0]; // Label 0 probability (Hot/Vehicle)
            float p1 = output->data.f[1]; // Label 1 probability (Rain/Flood)
            float p2 = output->data.f[2]; // Label 2 probability (Normal)

            // Find the highest probability
            int predicted_label = 0;
            float max_prob = p0;

            if (p1 > max_prob) { max_prob = p1; predicted_label = 1; }
            if (p2 > max_prob) { max_prob = p2; predicted_label = 2; }
            
            WS_LOG("--- 🤖Inference result: ---");
            WS_LOG("Prediction: [0]:" + String(p0*100, 2) + "% [1]:" + String(p1*100, 2) + "% [2]:" + String(p2*100, 2) + "%");
            WS_LOG("=>LABEL: " + String(predicted_label));

            // Control hardware (only when state changes AND not in Manual Mode)
            if (predicted_label != current_label) {
                current_label = predicted_label;

                if (current_label == 0) {
                    // LABEL 0: Turn on Fan
                    if (!isMotorManualMode) dc_motor_set(true);
                    if (!isRelayManualMode) relay_set(false);
                } 
                else if (current_label == 1) {
                    // LABEL 1: Turn on Relay
                    if (!isMotorManualMode) dc_motor_set(false);
                    if (!isRelayManualMode) relay_set(true);
                } 
                else {
                    // LABEL 2: Turn off all
                    if (!isMotorManualMode) dc_motor_set(false);
                    if (!isRelayManualMode) relay_set(false);
                }
            }
        }

        if (current_label == 0) {
            // Yellow 500ms -> Red 500ms
            set_all_leds(255, 255, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
            set_all_leds(255, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        } 
        else if (current_label == 1) {
            // Green 200ms -> Orange 200ms 
            set_all_leds(0, 255, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
            set_all_leds(255, 128, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        } 
        else if (current_label == 2) {
            // Static blue
            set_all_leds(0, 0, 255);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else {
            // LED off
            set_all_leds(0, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}