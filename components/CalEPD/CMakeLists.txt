# Add only the display srcs you are going to use if you want to keep compiled srcs to a minimum:
set(srcs 
    # Pending for more testing:
    "models/gdeh0213b73.cpp"
    # Common base classes
    "epd.cpp"
    "epd7color.cpp"
    "epdspi.cpp"
    "epd4spi.cpp"
    "epdParallel.cpp"
    )

idf_component_register(SRCS ${srcs}      
                    REQUIRES "Adafruit-GFX"

                    INCLUDE_DIRS "include"
)

