FetchContent_Declare(
    pico_sdk
    GIT_REPOSITORY  https://github.com/raspberrypi/pico-sdk.git
    GIT_TAG         main
)
FetchContent_GetProperties(pico_sdk)
if(NOT pico_sdk_POPULATED)
	FetchContent_Populate(pico_sdk)
endif()