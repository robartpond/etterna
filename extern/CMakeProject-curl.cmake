
#add_subdirectory(curl-7.55.1)
#set_property(TARGET "CURL" PROPERTY FOLDER "External Libraries")

set(CURL_DIR "${SM_EXTERN_DIR}/curl-7.55.1")
file(GLOB CURL_C "${CURL_DIR}/src/*.c")
file(GLOB CURL_H "${CURL_DIR}/src/*.h")
#file(GLOB_RECURSE CURLLIB_C "${CURL_DIR}/lib/*.c")
#file(GLOB_RECURSE CURLLIB_H "${CURL_DIR}/lib/*.h")
file(GLOB CURLLIB_C "${CURL_DIR}/lib/*.c")
file(GLOB CURLLIB_H "${CURL_DIR}/lib/*.h")
file(GLOB CURLLIB_VAUTH_C "${CURL_DIR}/lib/vauth/*.c")
file(GLOB CURLLIB_VAUTH_H "${CURL_DIR}/lib/vauth/*.h")
file(GLOB CURLLIB_VTLS_C "${CURL_DIR}/lib/vtls/*.c")
file(GLOB CURLLIB_VTLS_H "${CURL_DIR}/lib/vtls/*.h")

source_group("curl files" FILES ${CURL_C} ${CURL_H} ${CURLLIB_C} ${CURLLIB_H})

add_library("CURL" STATIC ${CURL_C} ${CURL_H} ${CURLLIB_C} ${CURLLIB_H} ${CURLLIB_VTLS_C} ${CURLLIB_VTLS_H} ${CURLLIB_VAUTH_C} ${CURLLIB_VAUTH_H})

set_property(TARGET "CURL" PROPERTY FOLDER "External Libraries")
set(CURL_INCLUDE_DIR "${CURL_DIR}/include")

#target_include_directories("CURL" PUBLIC "${CURL_INCLUDE_DIR}")
target_include_directories("CURL" PUBLIC "${CURL_DIR}/lib")

sm_add_compile_definition("CURL" CURL_STATICLIB)
sm_add_compile_definition("CURL" CURL_STATIC)