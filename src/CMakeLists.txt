
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets Concurrent)

add_executable(VDS 
	main.cpp
	main_window.h
	main_window.cpp

	common/vdtk_helper_functions.h
	
	fileio/export_item.h
	fileio/export_item.cpp
	fileio/export_image_series_dialog.h
	fileio/export_image_series_dialog.cpp
	fileio/export_raw_3D_dialog.h
	fileio/export_raw_3D_dialog.cpp
	fileio/import_item.h
	fileio/import_item.cpp
	fileio/import_item_list.h
	fileio/import_item_list.cpp
	fileio/import_raw_3D_dialog.h
	fileio/import_raw_3D_dialog.cpp
	fileio/import_binary_slices_dialog.h
	fileio/import_binary_slices_dialog.cpp
	
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

	tools/resize_volume_data.h
	tools/resize_volume_data.cpp

	widgets/expandable_section_widget.h
	widgets/expandable_section_widget.cpp
	widgets/histogram_view_GL.h
	widgets/histogram_view_GL.cpp
	widgets/slice_view_GL.h
	widgets/slice_view_GL.cpp
	widgets/volume_view_GL.h
	widgets/volume_view_GL.cpp
)


set(UI_FILES
	main_window.ui
)

set(RESOURCE_FILES
	qt_resources/resources.qrc
	qt_resources/stylesheets/style.qss)

qt5_wrap_ui(UI_HDRS ${UI_FILES})

target_sources(VDS
    PRIVATE
	${UI_FILES}
	${UI_HDRS}
	${RESOURCE_FILES}
)

replicate_directory_structure(VDS)

target_link_libraries(VDS PRIVATE vdtk_lib Threads::Threads Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Concurrent)


# set C++ language standard to c++17
target_compile_features(VDS PRIVATE cxx_std_17)
