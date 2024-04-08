idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ".")
                    
add_subdirectory(Drivers)
target_link_libraries(${COMPONENT_LIB} PUBLIC Drivers)