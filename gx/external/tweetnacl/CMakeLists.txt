cmake_minimum_required(VERSION 3.20)

project(TweetNaCl)

add_library(tweetnacl STATIC
        src/tweetnacl.c
        )

target_include_directories(tweetnacl PUBLIC include/)
