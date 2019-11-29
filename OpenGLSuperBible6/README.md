# OpenGL SuperBible 6 源码笔记
-------------------------------------------
## 02_simpleclear
* 使用 glClearBufferfv 清空颜色缓存和深度缓存

## 02_singlepoint
* 绘制单个点
* glPointSize() / glDrawArrays()

## 02_singletri
* 绘制单个三角形
* glDrawArrays()

## 03_fragcolorfrompos
* gl_FragCoord 在片段着色器中作为片段坐标

## 03_movingtri
* glVertexAttrib4fv --- 设置顶点属性
	// 着色器设置位置为 0 的属性
	layout (location = 0) in vec4 offset;

## 03_tessellatedgstri
* 使用几何着色器和分形着色器
* tcs
	- gl_InvocationID 在一条绘制命令中第几次调用
	- gl_TessLevelInner/gl_TessLevelOuter 设置其值
	- gl_out/gl_in
* tes
	- 求解 gl_Position
	- gl_TessCoord 分形坐标
	- gl_in 输入数组
* 几何着色器
	- layout (triangles) in
	- layout (points, max_vertices = 3) out

## 03_tessellatedtri
* 使用分形着色器
* glDrawArrays() 及 GL_PATCHES 参数绘制用于 tessellate 的 patch.

## 05_alienrain
* 演示了如何使用了纹理数组, texture 函数使用三维坐标
* 演示了如何使用 uniform buffer object, ubo.
* 本例使用标准布局, 绑定至1, 内容为结构体数组
```
struct droplet_t
{
    float x_offset;
    float y_offset;
    float orientation;
    float unused;
};

layout (std140, binding = 1) uniform droplets
{
    droplet_t droplet[256];
};
```
* 应用中创建 UBO
```
glGenBuffers(1, &rain_buffer);
glBindBuffer(GL_UNIFORM_BUFFER, rain_buffer);
glBufferData(GL_UNIFORM_BUFFER, 256 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
```
* 应用中设置 UBO 的值
```
// 使用 glBindBufferBase 将 GL_UNIFORM_BUFFER 的 buffer 绑定至一个 uniform buffer 绑定点.
glBindBufferBase(GL_UNIFORM_BUFFER, 1, rain_buffer);
// 使用 glMapBufferRange 映射一段范围的 buffer.
glm::vec4 * droplet = (glm::vec4 *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, 256 * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
for (int i = 0; i < 256; i++)
{
	droplet[i][0] = droplet_x_offset[i];
	droplet[i][1] = 2.0f - fmodf((t + float(i)) * droplet_fall_speed[i], 4.31f);
	droplet[i][2] = t * droplet_rot_speed[i];
	droplet[i][3] = 0.0f;
}
glUnmapBuffer(GL_UNIFORM_BUFFER);
```
* 创建 uniform block buffer, 传递批量数据
