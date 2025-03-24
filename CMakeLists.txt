cmake_minimum_required(VERSION 3.10.0)
project(VulkanTest VERSION 0.1.0 LANGUAGES C CXX)
find_package(Vulkan REQUIRED)

add_subdirectory(src)

add_executable(VulkanTest gameObject.cpp Main.cpp)

target_include_directories(VulkanTest PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(VulkanTest PRIVATE ${Vulkan_LIBRARIES})
target_link_libraries(VulkanTest PRIVATE src)

include(CTest)
enable_testing()

