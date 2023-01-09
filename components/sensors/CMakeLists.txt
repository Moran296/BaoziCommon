idf_component_register(SRC_DIRS "." "./pir" "./dht"
                    INCLUDE_DIRS "." "./pir" "./dht"
                    REQUIRES utilities network drivers homeassistant
                    PRIV_REQUIRES esp_timer)