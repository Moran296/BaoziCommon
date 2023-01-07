idf_component_register(SRC_DIRS "."
                    INCLUDE_DIRS "." 
                    REQUIRES drivers utilities esp_wifi mqtt
                    PRIV_REQUIRES nvs_flash) 