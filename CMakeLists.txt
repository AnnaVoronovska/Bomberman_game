cmake_minimum_required(VERSION 3.16)
project(Bomberman LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Logic library: bevat ALLE spelregels, GEEN SFML ----
# Kan in theorie gecompileerd worden zonder dat SFML geinstalleerd is.
add_library(bomberman_logic STATIC
        src/backend/Core.cpp
        src/backend/Entities.cpp
        src/backend/Score.cpp
        src/backend/World.cpp
        src/backend/Command.cpp
        src/backend/Visitor.cpp
)
target_include_directories(bomberman_logic PUBLIC src/backend)

# ---- Representatie: SFML-venster, Views, main-lus ----
find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

add_executable(bomberman
        src/frontend/Views.cpp
        src/frontend/ConcreteFactory.cpp
        src/frontend/AudioManager.cpp
        src/frontend/Game.cpp
        src/frontend/main.cpp
)
target_include_directories(bomberman PRIVATE src/frontend)
target_link_libraries(bomberman PRIVATE bomberman_logic sfml-graphics sfml-window sfml-system sfml-audio)