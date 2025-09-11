add_subdirectory(dr_libs)
add_subdirectory(emhash)
add_subdirectory(enet)
add_subdirectory(entt)
add_subdirectory(fastnoiselite)
add_subdirectory(glad)
add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(imgui)
add_subdirectory(miniz)
add_subdirectory(parson)
add_subdirectory(physfs)
add_subdirectory(salad)
add_subdirectory(spdlog)
add_subdirectory(stb)
add_subdirectory(thread_pool)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_target(thirdparty_txt ALL
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/thirdparty.py" "${CMAKE_CURRENT_LIST_DIR}"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/thirdparty.txt"
    DEPENDS "${CMAKE_CURRENT_LIST_DIR}/thirdparty.py"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/thirdparty.txt" DESTINATION ".")
