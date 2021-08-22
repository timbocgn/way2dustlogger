idf_component_register(SRCS "main.cpp" "ESP32_SHT1x.cpp" "rest_server.cpp" "sensor_manager.cpp" "config_manager.cpp" "infomanager.cpp" "mqtt_manager.cpp"
                    INCLUDE_DIRS ".")


set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../front/webapp")
if(EXISTS ${WEB_SRC_DIR}/dist)
    spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
else()
    message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
endif()