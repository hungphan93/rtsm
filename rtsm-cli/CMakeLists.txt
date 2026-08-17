add_executable(rtsm-cli main.cpp)

target_sources(rtsm-cli
	PRIVATE FILE_SET CXX_MODULES FILES
	system_monitor_view_cli.ixx
)

target_link_libraries(rtsm-cli PRIVATE RTSM::Core)

install(TARGETS rtsm-cli DESTINATION bin COMPONENT rtsm-cli)
