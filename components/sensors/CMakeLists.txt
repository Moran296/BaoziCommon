idf_component_register(SRC_DIRS "." "./pir" "./dht" "./bh1750" "./presence"
                    INCLUDE_DIRS "." "./pir" "./dht" "./bh1750" "./presence"
                    REQUIRES utilities network drivers homeassistant
                    PRIV_REQUIRES esp_timer)