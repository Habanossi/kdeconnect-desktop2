#SET(CMAKE_CXX_FLAGS "-std=c++11 -lfftw3 -lm -lasound -o")

#SET(fingerPrint_SRCS 
#fingerPrint.cpp
#AudiorŔead.cpp)

#link_directories(/usr/lib)

#target_link_libraries(fingerPrint
 #   kdeconnectcore
 #   Qt5::DBus
 #   KF5::I18n
#	asound
#	boost_program_options
#	python2.7
#	fftw3
#)

#find_package(ALSA REQUIRED)
#if(ALSA FOUND)
#	include_directories(${ALSA_INCLUDE_DIRS})
#	target_link_libraries(${APP_NAME} ${ALSA_LIBRARIES})
#endif(ALSA_FOUND)
