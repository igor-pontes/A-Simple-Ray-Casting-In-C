#include <stdio.h>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shaders.h"
#include "types.h"
#include "ray.h"

const int wWidth = 800;
const int wHeight = 700;

float mousePosition[2];

static void SceneRender();
static void ProcessInput(GLFWwindow* window);
static void ProcessMousePosition(GLFWwindow* window, double xpos, double ypos);

int main(void)
{
	GLFWwindow* window;
	if(!glfwInit())
		return -1;	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_RESIZABLE, False);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, True);
#endif
	window = glfwCreateWindow(wWidth, wHeight, "Simple Ray Casting in C", NULL, NULL);
	
	if(!window)
	{
		printf("Error trying to create window. \n");
		glfwTerminate();
		return -1;
	}	

	glfwMakeContextCurrent(window);
	
	if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD. \n");
		return -1;
	}

	glViewport(0,0,wWidth, wHeight);
	
	//----
	unsigned int VAO[3], VBO[3], EBO, vertexShader, fragmentShader, shaderProgram;
	int success;
	char infoLog[512];
	// Defining shaders and linking them
	// Compiling Vertex Shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	
	if(!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("%s \n",infoLog);
	}

	// Compiling Fragment Shader
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	
	if(!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("%s \n",infoLog);
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	
	if(!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("%s \n",infoLog);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Assigning ray and square vertices
	float r_vertices[] = {
        	0.0f, 0.0f, 0.0f, 
        	0.0f, 0.0f, 0.0f,
        	0.0f, 0.0f, 0.0f 
	};
	
	float s_vertices[] = {
		0.3f, 0.3f, 0.0f, // bottom left
	       	0.3f, 0.5f, 0.0f, // top left
		0.4f, 0.3f, 0.0f, // bottom right
		0.4f, 0.5f, 0.0f // top right
	};
	
	float p_vertices[] = {
		0.0f, 0.0f, 0.0f
	};

	Rect re1 = {.position.x = wWidth*1.3f/2, .position.y = wHeight*1.3f/2, .size.x = wWidth*1.4f/2 - wWidth*1.3f/2, .size.y =  wHeight*1.5f/2 - wHeight*1.3f/2 };
	
	re1.vertices[0].x = wWidth*(1.0f + s_vertices[0])/2;
	re1.vertices[0].y = wHeight * (1.0f + s_vertices[1])/2;
	re1.vertices[1].x = wWidth*(1.0f + s_vertices[3])/2;
	re1.vertices[1].y = wHeight * (1.0f + s_vertices[4])/2;
	re1.vertices[2].x = wWidth*(1.0f + s_vertices[6])/2;
	re1.vertices[2].y = wHeight * (1.0f + s_vertices[7])/2;
	re1.vertices[3].x = wWidth*(1.0f + s_vertices[9])/2;
	re1.vertices[3].y = wHeight * (1.0f + s_vertices[10])/2;

	unsigned int s_indices[] = {
		3, 2, 1,
		2, 0, 1
	};

	glGenVertexArrays(3, VAO);
	glGenBuffers(3, VBO);
	glGenBuffers(1, &EBO);
	

	// Ray (VAO[0])
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(r_vertices), r_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, False, 3 * sizeof(float), (void*)0);		
	// Enabling the attribute we just set up.
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// Square (VAO[1])
	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertices), s_vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_indices), s_indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, False, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	
	// Point (VAO[0])
	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(p_vertices), p_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, False, 3 * sizeof(float), (void*)0);		
	// Enabling the attribute we just set up.
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	//----
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);	
	
	while(!glfwWindowShouldClose(window))
	{	
		ProcessInput(window);	
		Ray r1;
		r1.position.x = (float) (r_vertices[0] + 1) * wWidth / 2;
		r1.position.y = (float) (r_vertices[1] + 1) * wHeight / 2;
		r1.direction.x = (mousePosition[0] - r1.position.x) / sqrt(pow(mousePosition[0] - r1.position.x, 2) + pow(mousePosition[1] - r1.position.y, 2));
		r1.direction.y = (mousePosition[1] - r1.position.y) / sqrt(pow(mousePosition[0] - r1.position.x, 2) + pow(mousePosition[1] - r1.position.y, 2));
		
		r_vertices[3] = ((r1.direction.x*200 + r1.position.x) * 2) / wWidth - 1;
		r_vertices[4] = ((r1.direction.y*200 + r1.position.y) * 2) / wHeight - 1;
		p_vertices[0] = 0.0f;
		p_vertices[1] = 0.0f;
		Vec2 col = CheckCollision(&r1, &re1);
		if(col.y != -1)
		{
			r_vertices[3] = (col.x * 2) / wWidth - 1;
			r_vertices[4] = (col.y * 2) / wHeight - 1;
			p_vertices[0] = (col.x * 2) / wWidth - 1;
			p_vertices[1] = (col.y * 2) / wHeight - 1;
			glPointSize(5.0f);

		}else{
			glPointSize(1.0f);
		}
		glfwSetCursorPosCallback(window, ProcessMousePosition);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(r_vertices), r_vertices);	
		glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(p_vertices), p_vertices);
		SceneRender(shaderProgram, VAO);	
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, VAO);
	glDeleteBuffers(1, VBO);
	glDeleteProgram(shaderProgram);
	glfwTerminate();
	return 0;
}
static void ProcessMousePosition(GLFWwindow* window, double xpos, double ypos)
{
	float mouseX_NDC = (float) ((2*xpos)/wWidth - 1);
	float mouseY_NDC = (float) -((2*ypos)/wHeight - 1);
	mousePosition[0] = (float) xpos;
	mousePosition[1] = (float) (mouseY_NDC + 1) * wHeight / 2;
}
static void ProcessInput(GLFWwindow* window)
{
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, True);
}
static void SceneRender(unsigned int shaderProgram, unsigned int *VAO)
{
	glClearColor(0.2f, 0.1f, 0.5f, 1.0fi);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgram);
	
	glBindVertexArray(VAO[0]);
	glDrawArrays(GL_LINE_STRIP, 0, 2);
	glBindVertexArray(VAO[1]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(VAO[2]);
	glDrawArrays(GL_POINTS, 0, 1);
}


