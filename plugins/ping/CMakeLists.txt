set(kdeconnect_ping_SRCS
    pingplugin.cpp
)

kdeconnect_add_plugin(kdeconnect_ping JSON kdeconnect_ping.json SOURCES ${kdeconnect_ping_SRCS})

find_library(fftw3 /home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/fftw-3.3.8)

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

target_link_libraries(kdeconnect_ping
    kdeconnectcore
    Qt5::DBus
    KF5::I18n
	asound
	fftw3
	Eigen3::Eigen
)

