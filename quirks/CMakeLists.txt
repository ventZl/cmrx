if (TARGET pico_stdlib)
    message(STATUS "Pico SDK detected!")

    if (NOT TARGET cmrx)
        message(FATAL_ERROR "CMRX target does not exist yet! Include this quirk after CMRX directory has been added!")
    endif()
    
    add_subdirectory(pico-sdk)
   
endif()
