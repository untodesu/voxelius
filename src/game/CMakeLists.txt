add_subdirectory(shared)

if(BUILD_VCLIENT)
    add_subdirectory(client)
endif()

if(BUILD_VSERVER)
    add_subdirectory(server)
endif()
