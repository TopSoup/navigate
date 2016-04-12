cmake_minimum_required(VERSION 3.5)
project(navigate)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -W -fexceptions")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -O2 -DNDEBUG")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/navigate)

add_definitions(
        -DWIN32
        -D_DEBUG
        -D_WINDOWS
        -D_MBCS
        -D_USRDLL
        -DAEE_SIMULATOR
)

include_directories(./navigate
        ./navigate/Location
        ./navigate/db
        ./navigate/controls
        d:/brew/sdk/inc
        d:/brew/sdk/sdk/inc)

set(SOURCE_FILES
    navigate/controls/gmenuctl.c
    navigate/controls/gmenuctl.h
    navigate/controls/TQueueList.c
    navigate/controls/TQueueList.h
    navigate/db/dbfunc.c
    navigate/db/dbfunc.h
    navigate/location/e_asin.c
    navigate/location/location.c
    navigate/location/location.h
    navigate/location/transform.c
    navigate/location/transform.h
    navigate/AEEAppGen.c
    navigate/AEEMediaUtil.c
    navigate/AEEModGen.c
    navigate/backimg.brx
    navigate/destinfowindow.c
    navigate/destinfowindow.h
    navigate/destlistfuctionwindow.c
    navigate/destlistfuctionwindow.h
    navigate/destlistwindow.c
    navigate/destlistwindow.h
    navigate/locationinfowindow.c
    navigate/locationinfowindow.h
    navigate/locinfowindow.c
    navigate/locinfowindow.h
    navigate/logicmacro.c
    navigate/logicmacro.h
    navigate/mainwindow.c
    navigate/mainwindow.h
    navigate/navigate.bid
    navigate/navigate.c
    navigate/navigate.h
    navigate/navigatedestwindow.c
    navigate/navigatedestwindow.h
    navigate/navigatepch.h
    navigate/navigatewindow.c
    navigate/navigatewindow.h
    navigate/newdestfuctionwindow.c
    navigate/newdestwherefuctionwindow.h
    navigate/newdestwindow.c
    navigate/newdestwindow.h
    navigate/soswindow.c
    navigate/soswindow.h
    navigate/sosrelativewindow.c
    navigate/sosrelativewindow.h
    navigate/sosinfowindow.c
    navigate/sosinfowindow.h
    navigate/textctlwindow.c
    navigate/textctlwindow.h
    navigate/wheredetailswindow.c
    navigate/wheredetailswindow.h
    navigate/wherefuctionwindow.c
    navigate/wherefuctionwindow.h
    navigate/wherewindow.c
    navigate/wherewindow.h
    navigate/wincontrol.c
    navigate/wincontrol.h
    navigate/window.c
    navigate/window.h)

function(gen_brh_and_bar BRX)
    message( ${BRX} )
    set(BREW_RES "d:/brew/sdk/tools/ResourceEditor/brewrc.exe")
    add_custom_command(
            OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/navigate/navigate.brh" "${CMAKE_CURRENT_SOURCE_DIR}/navigate/navigate.bar"
            COMMAND $(BREW_RES) -s -o CMAKE_CURRENT_SOURCE_DIR/navigate/navigate.bar -h CMAKE_CURRENT_SOURCE_DIR/navigate/navigate.brh ${BRX}
    )
endfunction()

#gen_brh_and_bar(${CMAKE_SOURCE_DIR}/navigate/navigate.brx)

add_library(navigate SHARED ${SOURCE_FILES})

#去掉lib前缀
set_target_properties(navigate PROPERTIES PREFIX "")