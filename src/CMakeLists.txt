
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)



add_executable(VDS 
	main.cpp
	main_window.ui
	main_window.h
	main_window.cpp

	common/vdtk_helper_functions.h
	
	fileio/import_item.h
	fileio/import_item.cpp
	fileio/import_item_list.h
	fileio/import_item_list.cpp
	fileio/import_raw_3D_dialog.h
	fileio/import_raw_3D_dialog.cpp

	
	widgets/volume_view_GL.h
	widgets/volume_view_GL.cpp
)

target_link_libraries(VDS PRIVATE vdtk_lib Qt5::Core Qt5::Gui Qt5::Widgets)


# set C++ language standard to c++17
target_compile_features(VDS PRIVATE cxx_std_17)
