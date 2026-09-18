cmake_minimum_required(VERSION 3.25)
project(NoCopyrightMod VERSION 1.0.0)

# Aseguramos la búsqueda del SDK de Geode provisto por la acción
find_package(Geode REQUIRED)

geode_target_include_directories(${PROJECT_NAME}
    PRIVATE src
)

geode_target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
)

geode_target_resources(${PROJECT_NAME}
    RESOURCES
        resources/sfh-index.min.json
        resources/auto-nong-index.min.json
)
