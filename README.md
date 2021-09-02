# Introduction
The engine is aimed for learning and education purposes. Designed with a step-by-step instruction on implementation of DirectX 11 rendering pipeline logic and with the foundation of [EnTT](https://github.com/skypjack/entt) Entity Component System architecture for performance.

## Features
* Controls- WASD for relative movement, spacebar and 'X' key for abolute movement on the same axis, left mouse click for rotation, 'Shift' key for sprint and 'R' for reset.
* Material support for diffuse and specular maps.
* Spawn multiple meshes with Assimp library support. Currently, the engine pushes single shader for animated mesh models with 52 bones, ~20k vertices with the bottleneck to iterate each bone based on elapsed time each frame. Other tests include 20k meshes * each having 4560 vertices i.e., a total of 91,200,000 vertices passed as a buffer to 1 shader material, running on Intel i5, GTX 1060 & 16GB RAM with 40 fps on average. 
* Point & Directional light support.
* Easy to expand engine. Add a feature by first creating a data component as a C++ struct in Components.h, then creating their entity in InitEngine.cpp and finally calling their system classes appropriately such as camera, mesh, rendering core etc., by passing the registry using EnTT library.
* Easy to organize and understand code, as an advantage to ECS model paradigm.

For more more information on the use of EnTT, visit [EnTT wiki](https://github.com/skypjack/entt/wiki).

# Test gallery
#### 1k meshes

![1k meshes](https://github.com/SButtan93-dev/DX11Starter_ECSEngine/blob/master/1k.gif)


#### 10k meshes

![10k meshes](https://github.com/SButtan93-dev/DX11Starter_ECSEngine/blob/master/10k.gif)

#### 20k meshes

![20k meshes](https://github.com/SButtan93-dev/DX11Starter_ECSEngine/blob/master/20k.gif)
</br>

#### 1k meshes with animation (current stage)

![1k meshes with animation](https://github.com/SButtan93-dev/DX11Starter_ECSEngine/blob/master/1k_AnimationMeshes_CloseUp.gif)
![1k meshes with animation](https://github.com/SButtan93-dev/DX11Starter_ECSEngine/blob/master/1k_AnimationMeshes.gif)
</br>

# Build
Clone and run the engine (.sln) on 'Release' mode VS2019. The dependencies are included.

# License
MIT.

# Dependencies
* DirectX11 graphics API as rendering core to communicate with GPU.
* [EnTT](https://github.com/skypjack/entt) architecture for performance.
* [NuGet](https://www.nuget.org/packages/directxtk_desktop_2017/) for converting textures.
* [Assimp](https://www.assimp.org/) with mesh loading and animation support.
