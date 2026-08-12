cmake_minimum_required(VERSION 3.16)
project(BombermanTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- GoogleTest: eerst kijken of het al systeem-breed geïnstalleerd is
# (sudo apt install libgtest-dev), anders via internet ophalen. Zo werkt dit
# ook zonder internetverbinding, zolang libgtest-dev geïnstalleerd is.
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# ---- Logic-library opnieuw gecompileerd, rechtstreeks vanuit de bron ----
# Bewust GEEN afhankelijkheid van het hoofd-CMakeLists.txt (dat SFML vereist):
# zo kan je de tests draaien zonder SFML te installeren.
set(BACKEND_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/backend)

add_library(bomberman_logic_test STATIC
        ${BACKEND_DIR}/Core.cpp
        ${BACKEND_DIR}/Entities.cpp
        ${BACKEND_DIR}/Score.cpp
        ${BACKEND_DIR}/World.cpp
        ${BACKEND_DIR}/Command.cpp
        ${BACKEND_DIR}/Visitor.cpp
)
target_include_directories(bomberman_logic_test PUBLIC ${BACKEND_DIR})

enable_testing()

add_executable(bomberman_tests
        TestFactory.h
        test_entities.cpp
        test_score.cpp
        test_world.cpp
        test_command.cpp
        test_visitor.cpp
)
target_link_libraries(bomberman_tests PRIVATE bomberman_logic_test GTest::gtest GTest::gtest_main)
target_include_directories(bomberman_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(bomberman_tests)