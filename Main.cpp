//std
#include <iostream>
#include <vector>
#include <array>
#include <future>
#include <cstdlib>
#include <stdexcept>


#include "src/VulkanTest/vulkanApp.h"
//Remove When Done
#include "timeCheck.h"
int main() {
    lve::vulkanApp app{};

    try {
        app.Run();
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
    return 0;
}