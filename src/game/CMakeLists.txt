if(BUILD_VCLIENT)
    add_subdirectory(client)
endif()

if(BUILD_VSERVER)
    add_subdirectory(server)
endif()

if(BUILD_VCLIENT OR BUILD_VSERVER)
    add_subdirectory(shared)
endif()
