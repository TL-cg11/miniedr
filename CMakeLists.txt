# CMake 프로그램의 최소 버전
cmake_minimum_required(VERSION 3.31)

# 프로젝트 정보
project (
	miniedr
	VERSION 0.0.1
	DESCRIPTION "Mini-EDR"
	LANGUAGES CXX)

# C++17 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)		# 필수 설정
set(CMAKE_CXX_EXTENSIONS OFF)			# 컴파일러 확장 기능 비활성화

if(MSVC)
    add_compile_options("/utf-8")
endif()

find_package(spdlog CONFIG REQUIRED)

# 실행 파일 생성 
add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)

# spdlog 타겟 링크 설정
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE spdlog::spdlog)

