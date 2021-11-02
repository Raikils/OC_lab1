add_executable(lab lab1.c)

target_include_directories(lab PUBLIC
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(lab PUBLIC lab1)
