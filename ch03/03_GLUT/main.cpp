#include <GL/glut.h> // 引入 GLUT 库

// 渲染函数
void render()
{
	glClear(GL_COLOR_BUFFER_BIT); // 清空颜色缓冲区

	glBegin(GL_TRIANGLES); // 开始绘制三角形
	glColor3f(1.0, 0.0, 0.0); // 设置顶点颜色（红色）
	glVertex2f(-0.6, -0.5); // 设置第一个顶点坐标
	glColor3f(0.0, 1.0, 0.0); // 设置顶点颜色（绿色）
	glVertex2f(0.6, -0.5); // 设置第二个顶点坐标
	glColor3f(0.0, 0.0, 1.0); // 设置顶点颜色（蓝色）
	glVertex2f(0.0, 0.5); // 设置第三个顶点坐标
	glEnd(); // 结束绘制

	glFlush(); // 刷新绘图命令
}
    
int main(int argc, char** argv)
{
	glutInit(&argc, argv); // 初始化 GLUT
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // 设置显示模式
	glutInitWindowSize(400, 300); // 设置窗口大小
	glutCreateWindow("OpenGL Triangle"); // 创建窗口并设置标题
	glClearColor(0.0, 0.0, 0.0, 0.0); // 设置清空颜色缓冲区时的颜色

	glutDisplayFunc(render); // 设置渲染函数

	glutMainLoop(); // 进入主循环
	return 0;
}