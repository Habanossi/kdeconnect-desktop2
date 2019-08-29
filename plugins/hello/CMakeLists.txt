set(kdeconnect_hello_SRCS
    helloplugin.cpp
)

kdeconnect_add_plugin(kdeconnect_hello JSON kdeconnect_hello.json SOURCES ${kdeconnect_hello_SRCS})

target_link_libraries(kdeconnect_hello
    kdeconnectcore
    Qt5::DBus
    KF5::I18n
)
