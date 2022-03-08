cmake_minimum_required(VERSION 3.18)
project(Dx11ShareTest)
file(GLOB srcFiles CONFIGURE_DEPENDS src/*)

add_subdirectory(third-party/SDL)
add_executable(dx11ShareTest ${srcFiles})
target_link_libraries(dx11ShareTest PRIVATE
	SDL2
	SDL2main
	d3d11
)

