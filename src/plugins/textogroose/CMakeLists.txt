cmake_minimum_required (VERSION 2.8)
project (leechcraft_textogroose)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	textogroose.cpp
	apiobject.cpp
	)
#CreateTrs("textogroose" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("textogroose" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_textogroose SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_textogroose
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_textogroose DESTINATION ${LC_PLUGINS_DEST})
