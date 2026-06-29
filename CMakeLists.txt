cmake_minimum_required(VERSION 3.19)
project(QtVulkanRayTracing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add shaders directory
include_directories(shaders)
file(GLOB SHADERS "shaders/shader.*")

# Add header directory
include_directories(header)
file(GLOB HEADER "header/*.h")

# Add src directory
file(GLOB SOURCES "src/*.cpp")
file(GLOB UI_FILES "src/*.ui")

# Add glm
add_subdirectory(externals/glm)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME}
    WIN32 MACOSX_BUNDLE
    ${HEADER}
    ${SOURCES}
    ${UI_FILES}
    ${SHADERS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
        glm
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
