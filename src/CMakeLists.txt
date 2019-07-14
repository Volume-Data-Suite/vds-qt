
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)

add_executable(VDS 
	main.cpp
	main_window.h
	main_window.cpp

	common/vdtk_helper_functions.h
	
	fileio/import_item.h
	fileio/import_item.cpp
	fileio/import_item_list.h
	fileio/import_item_list.cpp
	fileio/import_raw_3D_dialog.h
	fileio/import_raw_3D_dialog.cpp
	
	renderer/shader/shader_code_constants.h
	renderer/shader/shader_settings.h
	renderer/shader/shader_generator.h
	renderer/shader/shader_generator.cpp
	renderer/textures/noise_texture_2D.h
	renderer/textures/noise_texture_2D.cpp
	renderer/textures/volume_data_3D_texture.h
	renderer/textures/volume_data_3D_texture.cpp
	renderer/textures/texture_units.h
	renderer/raycast_renderer_gl.h
	renderer/raycast_renderer_gl.cpp

	widgets/volume_view_GL.h
	widgets/volume_view_GL.cpp
)


set(UI_FILES
	main_window.ui
)

qt5_wrap_ui(UI_HDRS ${UI_FILES})

target_sources(VDS
    PRIVATE
	${UI_FILES}
	${UI_HDRS}
)

replicate_directory_structure(VDS)

target_link_libraries(VDS PRIVATE vdtk_lib Qt5::Core Qt5::Gui Qt5::Widgets)


# set C++ language standard to c++17
target_compile_features(VDS PRIVATE cxx_std_17)
