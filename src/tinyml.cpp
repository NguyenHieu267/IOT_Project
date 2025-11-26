#include "tinyml.h"
#include "global.h"
#include "sensor_bus.h"
#include "DC_motor.h"
#include "relay_control.h"
#include "4_led_rgb.h"

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
    Serial.println("TensorFlow Lite Init....");
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

    Serial.println("TensorFlow Lite Micro initialized on ESP32.");
}

void tiny_ml_task(void *pvParameters)
{

    SensorBus *bus = static_cast<SensorBus *>(pvParameters);
    if (bus == nullptr)
    {
        Serial.println("[tiny_ml_task] Missing SensorBus pointer");
        vTaskDelete(nullptr);
    }

    setupTinyML();

    while (1)
    {
        // Read sensor data 
        SensorReading reading{};
        if (!sensor_bus_peek(bus, reading, pdMS_TO_TICKS(1000)))
        {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }
        float temp = reading.temperature;
        float humi = reading.humidity;
        
        // Prepare input data for TinyML model
        input->data.f[0] = temp;
        input->data.f[1] = humi;

        // Run inference
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk)
        {
            error_reporter->Report("Invoke failed");
            return;
        }

        // Get and process output
        float result = output->data.f[0];
        Serial.print("Inference result: ");
        Serial.println(result);

        static int counter = 0;
        static int mode = -1; // -1 = chưa set gì, 0 = motor mode, 1 = relay mode

        // Xác định mode theo result
        if (result <= 0.5f) {
            if (mode != 0) {
                // Chỉ chạy 1 lần khi đổi mode
                mode = 0;
                dc_motor_set(true);     // bật quạt liên tục
                relay_set(false);       // tắt relay
                counter = 0;            // reset hiệu ứng LED
            }
        } 
        else {
            if (mode != 1) {
                // Chỉ chạy 1 lần khi đổi mode
                mode = 1;
                relay_set(true);        // bật relay liên tục
                dc_motor_set(false);    // tắt quạt
                counter = 0;            // reset hiệu ứng LED
            }
        }

        // ==========================
        //      HIỆU ỨNG LED
        // ==========================

        if (mode == 0) {
            // MODE 0: MOTOR — LED vàng → đỏ lặp theo chu kỳ
            if (counter < 1000) {
                set_all_leds(255, 255, 0); // vàng
            } else if (counter < 2000) {
                set_all_leds(255, 0, 0);   // đỏ
            } else {
                counter = 0;               // lặp lại chu kỳ
            }
        }

        if (mode == 1) {
            // MODE 1: RELAY — LED xanh → cam nhấp nháy
            if (counter < 500) {
                set_all_leds(0, 255, 0);   // xanh lá
            } 
            else {
                int t = counter - 500;
                int cycle = t % 500;

                if (cycle < 300) {
                    set_all_leds(255, 128, 0); // cam
                } else {
                    set_all_leds(0, 0, 0);     // tắt
                }
            }

            if (counter > 2000) counter = 0; // lặp lại chu kỳ
        }

        // Tick nhỏ cho mượt
        vTaskDelay(1);
        counter++;


        vTaskDelay(5000);
    }
}