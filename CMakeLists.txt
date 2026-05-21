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
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(unofficial-libyara CONFIG REQUIRED)

# 실행 파일 생성 
add_executable(${CMAKE_PROJECT_NAME} 
 src/main.cpp
 src/core/Logger.hpp
 src/core/Logger.cpp
 src/core/Event.hpp
 src/core/Event.cpp
 src/core/EventBus.hpp
 src/core/EventBus.cpp
 src/storage/Database.hpp
 src/storage/Database.cpp
 src/detection/IDetector.hpp
 src/detection/HashDetector.hpp
 src/detection/HashDetector.cpp
 src/detection/YaraDetector.hpp
 src/detection/YaraDetector.cpp
 src/scanner/FileScanner.hpp
 src/scanner/FileScanner.cpp
 src/scanner/DirectoryMonitor.hpp
 src/scanner/DirectoryMonitor.cpp
 src/core/StringUtil.hpp
 src/core/StringUtil.cpp
 src/quarantine/Quarantine.hpp
 src/quarantine/Quarantine.cpp)

# spdlog 타겟 링크 설정
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE bcrypt)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE unofficial::libyara::libyara)

# include 경로 설정
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src)

