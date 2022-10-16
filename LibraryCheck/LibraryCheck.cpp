#include <iostream>
#include <filesystem>

int main() {
	// Check for assimp 
	if (!std::filesystem::exists("../x64/Debug/assimp-vc143-mtd.dll")) {
		std::filesystem::copy("../Dependencies/assimp/lib/assimp-vc143-mtd.dll", "../x64/Debug/");
	};
	// Check for freeimage
	if (!std::filesystem::exists("../x64/Debug/FreeImaged.dll")) {
		std::filesystem::copy("../Dependencies/freeimage/lib/FreeImaged.dll", "../x64/Debug/");
	};
}

