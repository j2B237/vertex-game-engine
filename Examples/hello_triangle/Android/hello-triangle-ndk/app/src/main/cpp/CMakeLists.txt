cmake_minimum_required(VERSION 3.22.1)

project(native-lib)

add_library(
    native-lib
    SHARED
    ${CMAKE_SOURCE_DIR}/native-lib.c
)

add_library(
    app-glue
    STATIC 
    ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

add_library(
    VGEUtils_android
    STATIC
    ${CMAKE_SOURCE_DIR}/../../../../../../../../RenderSystems/GLES2/src/Android/VGEUtils_android.c
    ${CMAKE_SOURCE_DIR}/../../../../../../../../RenderSystems/GLES2/src/VGEUtils.c
)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

include_directories(
    ${CMAKE_SOURCE_DIR}//../../../../../../../../RenderSystems/GLES2/include
)

find_library(
    log-lib
    log
)

find_library(
    egl-lib
    EGL
)

find_library(
    gles-lib
    GLESv3
)

find_library(
    android-lib
    android
)

target_include_directories(
    app-glue
    PUBLIC
    ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/
)

target_include_directories(
    VGEUtils_android
    PRIVATE
    ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/
)

# IMPORTANT : Ces options reglent le probleme d'alignement des applications natives
# sur le format de taille de page sur Android qui est passe de 4Ko a 16 Ko.
# L'option -u ANativeActivity_onCreate permet de referencer explicitement un point d'entree 
# au moment de l'edition de liens. ANativeActivity_onCreate est exporte par libnative-lib.so

target_link_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE 
    "-u"
    "ANativeActivity_onCreate"
    "-Wl,-z,max-page-size=16384"
    "-Wl,-z,common-page-size=16384"
)

target_link_libraries(
    native-lib 
    app-glue
    VGEUtils_android
    ${log-lib}
    ${egl-lib}
    ${gles-lib}
    ${android-lib}
)