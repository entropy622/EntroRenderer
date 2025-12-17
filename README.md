学 **LearnOpenGL** 的时候做的一个小渲染器

除教程外的部分，额外写了一个简单的toon shader。
<img width="877" height="674" alt="image" src="https://github.com/user-attachments/assets/ca5433aa-d858-4605-8c97-565b264b8f58" />

WWASD鼠标移动，上下左右 LShift LCtrl快捷控制光源。

如果对你有帮助，请随意参考。

## 编译问题
如果出现了确实dll的情况，可能需要把
`cmake-build-debug/_deps/assimp-build/bin/libassimp-5d.dll` 复制到 `cmake-build-debug` 的根目录。不过一般来说Cmake会自动做这件事。 

## LearnOpenGL 地址：
https://learnopengl-cn.github.io   真的是很好的教程。
