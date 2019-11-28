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
* 创建 uniform block buffer, 传递批量数据
