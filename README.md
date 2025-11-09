# Arena Souls setup

This is an OpenGL combat game project 

# Visual Studio 2022 Dev Setup
- Open **ArenaSouls.sln** file in VS 2022
- Right click the project name -> **Properties** -> **Configuration Properties/VC++ Directories**
- Make sure that **"Include Directories"** and **"Library Directories"** are set to our **include** and **lib** in **Libraries** path
  
  <img width="987" height="674" alt="image" src="https://github.com/user-attachments/assets/9fda657e-7e2a-4491-a800-ee3835c1ebd9" />

- Go to Configuration **Properties/Linker/General** and make sure that **Additional Library Directory** is set like the picture below

  <img width="989" height="671" alt="image" src="https://github.com/user-attachments/assets/bba35232-4665-4066-a787-122542837eea" />

  
- Go to Configuration **Properties/Linker/Input** and make sure that **Additional Dependencies** is set like the list below

  - glfw3.lib
  - opengl32.lib
  - assimp.lib
  - freetype.lib
  - glew32s.lib
  - irrKlang.lib
  - SOIL.lib
  
  <img width="988" height="671" alt="image" src="https://github.com/user-attachments/assets/a4c32276-b90e-45d5-a9ff-37cb8731637d" />


- Finish!
