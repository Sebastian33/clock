idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ".")
                    
add_subdirectory(Drivers)
add_subdirectory(Tasks)
target_link_libraries(${COMPONENT_LIB} PUBLIC Drivers Tasks)