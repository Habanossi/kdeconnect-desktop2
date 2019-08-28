set(kdeconnect_privacy_SRCS
    privacyplugin.cpp
)

kdeconnect_add_plugin(kdeconnect_privacy JSON kdeconnect_privacy.json SOURCES ${kdeconnect_privacy_SRCS})

find_library(fftw3 /home/hermanni/kdeconnect-kde-1.3.4/plugins/privacy/fingerprinter/fftw-3.3.8)

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

target_link_libraries(kdeconnect_privacy
    kdeconnectcore
    Qt5::DBus
    KF5::I18n
	asound
	fftw3
	Eigen3::Eigen
)

