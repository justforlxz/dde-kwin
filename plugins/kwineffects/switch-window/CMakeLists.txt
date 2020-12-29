set(TARGET_NAME "switch-window")

find_package(KF5Config REQUIRED)

add_library(
    ${TARGET_NAME} MODULE
    switch-window.cpp
    main.cpp
)

target_link_libraries(
    ${TARGET_NAME}
    PUBLIC
    Qt5::Core
    Qt5::Gui
    PRIVATE
    KF5::CoreAddons
    KF5::WindowSystem
    KF5::ConfigCore
)

install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_PATH})
