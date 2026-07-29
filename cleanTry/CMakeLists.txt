cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(cleanTry)

target_sources(app PRIVATE
  src/main.c
  src/sysram.c
)

zephyr_linker_sources(SECTIONS sysram.ld)
