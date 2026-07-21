if(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
  zephyr_library()
  zephyr_library_sources(src/status_screen.c)
  # ZMK's app headers (zmk/display.h, widgets, events) are private to the app
  # target, so pull them in explicitly for this module.
  zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)
endif()
