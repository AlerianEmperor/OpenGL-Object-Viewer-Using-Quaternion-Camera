#include <gl\glew.h>
#include <gl\glfw3.h>
#include <gl\freeglut.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "trackball.h"
#include "model.h"
#include "Utility.h"


using namespace glm;

#define width 1024
#define height 768

bool MouseLeftPress = false;
bool MouseMiddlePress = false;
bool MouseRightPress = false;
float prev_quat[4];
float cur_quat[4];

double prevMouseX, prevMouseY;

GLuint program;
GLuint mvpLoc;
GLuint mvLoc;
GLuint nLoc;
GLuint vao;
GLuint vbo;

GLuint KdLoc, DiffuseLoc;
int num_mesh;

float m[4][4];

mat4 mMat;
mat4 vMat;
mat4 pMat;

float eye[3] = {0.0f, 0.0f, 3.0f};
float lookat[3] = { 0.0f, 0.0f, 0.0f };

//Light Loc

GLuint globalAmbLoc;
GLuint ambLoc;
GLuint diffLoc;
GLuint specLoc;
GLuint posLoc;

//Light Properties
float a = 0.2f;
float d = 1.0f;
float s = 2.0f;
float globalAmbient[4] = { a, a, a, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { d, d, d, 1.0f };
float lightSpecular[4] = { s, s, s, 1.0f };

//click func = mouse button call back
static void clickFunc(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			MouseLeftPress = true;
			trackball(prev_quat, 0.0f, 0.0f, 0.0f, 0.0f);
		}
		else if (action == GLFW_RELEASE)
			MouseLeftPress = false;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
			MouseRightPress = true;
		else if (action == GLFW_RELEASE)
			MouseRightPress = false;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		if (action == GLFW_PRESS)
			MouseMiddlePress = true;
		else if (action == GLFW_RELEASE)
			MouseMiddlePress = false;
	}
}

//motionFunc = cursor CallBack
static void motionFunc(GLFWwindow* window, double mouseX, double mouseY)
{
	float translate_Scale = 2.0f;
	if (MouseLeftPress)
	{
		trackball(prev_quat, (2.0f * prevMouseX - width) / (float)width,
			(height - 2.0f * prevMouseY) / (float)height,
			(2.0f * mouseX - width) / (float)width,
			(height - 2.0f * mouseY) / (float)height);
		add_quats(prev_quat, cur_quat, cur_quat);
	}
	else if (MouseRightPress)
	{
		float translate_x = (mouseX - prevMouseX) / (float)width;
		float translate_y = (mouseY - prevMouseY) / (float)height;
		eye[0] -= translate_x;
		eye[1] += translate_y;

		lookat[0] -= translate_x;
		lookat[1] += translate_y;
	}
	else if (MouseMiddlePress)
	{
		float translate_depth = translate_Scale * (mouseY - prevMouseY) / (float)height;
		eye[2] += translate_depth;
		lookat[2] += translate_depth;
	}
	prevMouseX = mouseX;
	prevMouseY = mouseY;
}

void init_light(GLuint program, vec3 light_position)
{
	float light_pos[3];
	light_pos[0] = light_position.x;
	light_pos[1] = light_position.y;
	light_pos[2] = light_position.z;

	globalAmbLoc = glGetUniformLocation(program, "globalAmbient");

	ambLoc = glGetUniformLocation(program, "light.ambient");
	diffLoc = glGetUniformLocation(program, "light.diffuse");
	specLoc = glGetUniformLocation(program, "light.specular");
	posLoc = glGetUniformLocation(program, "light.position");

	glProgramUniform4fv(program, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(program, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(program, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(program, specLoc, 1, lightSpecular);
	glProgramUniform3fv(program, posLoc, 1, light_pos);
}

void init_data(Model& model)
{
	trackball(cur_quat, 0.0f, 0.0f, 0.0f, 0.0f);
	mvpLoc = glGetUniformLocation(program, "mvp_matrix");
	mvLoc = glGetUniformLocation(program, "mv_matrix");
	nLoc = glGetUniformLocation(program, "normal_matrix");
	//hasMaskLoc = glGetUniformLocation(program, "HasMask");

	/*glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * 4, &model.vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	*/

	for (int i = 0; i < model.indices.size(); ++i)
	{
		glGenVertexArrays(1, &model.indices[i].vao);
		glBindVertexArray(model.indices[i].vao);

		if (model.mats[i].useTexture)
		{
			glGenBuffers(1, &model.indices[i].vbo_vertices);
			glBindBuffer(GL_ARRAY_BUFFER, model.indices[i].vbo_vertices);
			glBufferData(GL_ARRAY_BUFFER, model.indices[i].vertices.size() * sizeof(vec3), &model.indices[i].vertices[0], GL_STATIC_DRAW);

			glGenBuffers(1, &model.indices[i].vbo_texcoords);
			glBindBuffer(GL_ARRAY_BUFFER, model.indices[i].vbo_texcoords);
			glBufferData(GL_ARRAY_BUFFER, model.indices[i].texcoords.size() * sizeof(vec2), &model.indices[i].texcoords[0], GL_STATIC_DRAW);

			glGenBuffers(1, &model.indices[i].vbo_normals);
			glBindBuffer(GL_ARRAY_BUFFER, model.indices[i].vbo_normals);
			glBufferData(GL_ARRAY_BUFFER, model.indices[i].normals.size() * sizeof(vec3), &model.indices[i].normals[0], GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, model.indices[i].vbo_vertices);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, model.indices[i].vbo_normals);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, model.indices[i].vbo_texcoords);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}
		else
		{
			glGenBuffers(1, &model.indices[i].vbo_vertices);
			glBindBuffer(GL_ARRAY_BUFFER, model.indices[i].vbo_vertices);
			glBufferData(GL_ARRAY_BUFFER, model.indices[i].vertices.size() * sizeof(vec3), &model.indices[i].vertices[0], GL_STATIC_DRAW);

			glGenBuffers(1, &model.indices[i].vbo_normals);
			glBindBuffer(GL_ARRAY_BUFFER, model.indices[i].vbo_normals);
			glBufferData(GL_ARRAY_BUFFER, model.indices[i].normals.size() * sizeof(vec3), &model.indices[i].normals[0], GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, model.indices[i].vbo_vertices);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, model.indices[i].vbo_normals);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}

		glGenBuffers(1, &model.indices[i].IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indices[i].IBO);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices[i].index.size() * 4, &model.indices[i].index[0], GL_STATIC_DRAW);

		//glUniform1i(hasMaskLoc, model.mats[i].has_mask);
		
	}

	KdLoc = glGetUniformLocation(program, "useKd");
	DiffuseLoc = glGetUniformLocation(program, "Kd");
	num_mesh = model.indices.size();
}

static void Draw_Model(GLFWwindow*& window, Model& model)
{
	//mMat = curr_quat.RotationMatrix();
	mMat = mat4(m[0][0], m[0][1], m[0][2], m[0][3],
				m[1][0], m[1][1], m[1][2], m[1][3],
				m[2][0], m[2][1], m[2][2], m[2][3],
				m[3][0], m[3][1], m[3][2], m[3][3]);
	vMat = glm::lookAt(vec3(eye[0], eye[1], eye[2]), vec3(lookat[0], lookat[1], lookat[2]), vec3(0.0f, 1.0f, 0.0f));
	mat4 mvMat = vMat * mMat;
	mat4 mvpMat = pMat * mvMat;//vMat * mMat;
	mat4 nMat = transpose(inverse(mvMat));

	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, value_ptr(mvpMat));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, value_ptr(mvMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, value_ptr(nMat));

	//glBindVertexArray(vao);

	//int start = 0;
	//int num_mesh = model.indices.size();
	//#pragma omp parallel for
	for (int i = num_mesh - 1; i >= 0; --i)
	{
		glBindVertexArray(model.indices[i].vao);

		if (model.mats[i].useTexture)
		{
			glUniform1i(KdLoc, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, model.mats[i].Texture_Kd_Id);
		}
		else
		{
			glUniform1i(KdLoc, 1);
			vec3 Kd = model.mats[i].Kd;
			glUniform4f(DiffuseLoc, Kd.x, Kd.y, Kd.z, 1.0f);
		}

		//if (model.mats[i].has_mask)
		//{
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, model.mats[i].Texture_Mask_Id);
		//}
		//glUniform1i(diffuseLoc, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indices[i].IBO);
		glDrawElements(GL_TRIANGLES, model.indices[i].index.size(), GL_UNSIGNED_INT, 0);

		//int size = model.indices[i].ind.size();
		//glDrawArrays(GL_TRIANGLES, start, size / 2);
		//start += size / 2;
	}
	//glBindVertexArray(0);
}

void main()
{
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glewExperimental = GL_TRUE;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow* window = glfwCreateWindow(width, height, "ArcBall", NULL, NULL);

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	//glfwSetKeyCallback(window, KeysCallBack);
	glfwSetCursorPosCallback(window, motionFunc);
	glfwSetMouseButtonCallback(window, clickFunc);

	//glutInit(&argc, argv);

	//glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode

	//glutInitWindowSize(windowWidth, windowHeight);              // window size


	//getchar();
	mMat = glm::mat4(1.0);
	vMat = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0., 0., 0.), glm::vec3(0., 1., 0.));
	pMat = glm::perspective(70.0f, 4.0f / 3.0f, 0.1f, 100.0f);

	Model model("E:\\Models\\sibenik\\sibenik.obj");
	//Model model("E:\\Models\\crytek_sponza\\textures\\crytek_sponza.obj");
	//Model model("E:\\Models\\Bath_Room\\contemporary_bathroom_lux_bikini.obj");

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	Utility utils;
	program = utils.CreateProgram("vs.glsl", "fs.glsl");

	init_data(model);

	vec3 max_vector = model.max_vector;
	vec3 min_vector = model.min_vector;

	vec3 center = (max_vector + min_vector) * 0.5f;

	vec3 light_position = center + vec3(-5, 5, 0);//vec3(center.x+200, max_vector.y + 20.0f, center.z);

	init_light(program, light_position);

	glUseProgram(program);

	


	//Model model("E:\\Models\\bath_room\\textures\\salle_de_bain.obj");
	//Model model("E:\\Models\\crytek_sponza\\textures\\crytek_sponza.obj");
	//Model model("E:\\Models\\Living_Room\\living_room\\textures\\LivingRoom.obj");
	//init_data(model);

	
	

	glClearColor(0.5, 0.5, 0.95, 0.95);

	
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glViewport(0, 0, Width / 2, Height/2);

		build_rotmatrix(m, cur_quat);

		Draw_Model(window, model);

		//float t = glfwGetTime();

		/*light_position += vec3(10 * sinf(t), 10 * cosf(t), 0);

		float light_pos[3];
		light_pos[0] = light_position.x;
		light_pos[1] = light_position.y;
		light_pos[2] = light_position.z;

		glProgramUniform3fv(program, posLoc, 1, light_pos);*/

		glfwSwapBuffers(window);
		glfwPollEvents();
		//string str = "Pos: " + c
		//glfwSetWindowTitle(window, )
	}

	model.ClearMemory();

	glBindVertexArray(0);
	glBindBuffer(vbo, 0);
	for (int i = 0; i < model.indices.size(); ++i)
	{
		glBindBuffer(model.indices[i].IBO, 0);
	}
	for (int i = 0; i < model.mats.size(); ++i)
	{
		glDeleteTextures(1, &model.mats[i].Texture_Kd_Id);
	}


}
