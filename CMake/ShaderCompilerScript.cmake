message(STATUS "Will do some compiling now :3")

message(STATUS "File passed value is '${FILE}'")
message(STATUS ${CMAKE_SOURCE_DIR})

file(STRINGS ${FILE} commands)

foreach (command IN LISTS commands)
    message(STATUS "${command}")
    execute_process(COMMAND ${command})
endforeach ()