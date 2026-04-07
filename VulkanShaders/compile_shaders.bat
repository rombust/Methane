echo off

"C:\VulkanSDK\1.4.341.1\Bin\glslc.exe" game.vert -o game_vert_release_11.spv -O --target-env=vulkan1.1
"C:\VulkanSDK\1.4.341.1\Bin\glslc.exe" game.frag -o game_frag_release_11.spv -O --target-env=vulkan1.1

"C:\VulkanSDK\1.4.341.1\Bin\glslc.exe" game.vert -o game_vert_debug_11.spv -g -O --target-env=vulkan1.1
"C:\VulkanSDK\1.4.341.1\Bin\glslc.exe" game.frag -o game_frag_debug_11.spv -g -O --target-env=vulkan1.1

echo Use linux:  "for i in *.spv; do xxd -i $i >$i.h; done"
echo Put headers in "Sources\Game"

pause
