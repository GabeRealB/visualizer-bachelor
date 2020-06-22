set(TOOLS mdh2vis)

foreach (TOOL IN LISTS TOOLS)
    message(STATUS "Adding ${TOOL}")
    add_subdirectory(${TOOL})
endforeach ()