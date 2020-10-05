cmake_minimum_required(VERSION 3.16)

# This file doesn't work. ;_;

if(DEFINED CMAKE_GENERATOR AND NOT CMAKE_GENERATOR MATCHES "Ninja")
    message("[Warn]: Generator is already defined as ${CMAKE_GENERATOR}.")
else()
    set(CMAKE_GENERATOR "Ninja"
        CACHE INTERNAL "")
endif()