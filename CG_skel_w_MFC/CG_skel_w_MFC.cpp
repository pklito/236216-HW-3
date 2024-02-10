// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include "MeshModel.h"
#include <string>

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#define FILE_OPEN 1
#define MAIN_DEMO 2
#define MAIN_ABOUT 3

#define RESCALE_WINDOW_MENU_ITEM_UP 4
#define RESCALE_WINDOW_MENU_ITEM_DOWN 5
#define DRAW_NORMALS 1
#define HIDE_NORMALS 2
#define DRAW_BOUNDING_BOX 3
#define HIDE_BOUNDING_BOX 4

Scene* scene;
Renderer* renderer;
Camera* camera;
float m_time;

int last_x, last_y;
bool lb_down, rb_down, mb_down;

//----------------------------------------------------------------------------
// Callbacks

void display(void)
{
	//Call the scene and ask it to draw itself
	std::cout << "<FRAME>" << std::endl;
	m_time += 0.1;
	//camera->LookAt(vec3(1,m_time,1),vec3(-1,0,0),vec3(0,1,0));

	renderer->SetCameraTransformInverse(camera->getTransformInverse());
	renderer->SetProjection(camera->getProjection());

	scene->draw();
	std::cout << "<FRAME END>" << ::std::endl;
}

void reshape(int width, int height)
{
	//update the renderer's buffers
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033:
		exit(EXIT_SUCCESS);
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	//button = {GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON}
	//state = {GLUT_DOWN,GLUT_UP}

	//set down flags
	switch (button) {
	case GLUT_LEFT_BUTTON:
		lb_down = (state == GLUT_UP) ? 0 : 1;
		break;
	case GLUT_RIGHT_BUTTON:
		rb_down = (state == GLUT_UP) ? 0 : 1;
		break;
	case GLUT_MIDDLE_BUTTON:
		mb_down = (state == GLUT_UP) ? 0 : 1;
		break;
	}

	// add your code
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx = x - last_x;
	int dy = y - last_y;
	// update last x,y
	last_x = x;
	last_y = y;
}

void fileMenu(int id)
{
	switch (id)
	{
	case FILE_OPEN:
		CFileDialog dlg(TRUE, _T(".obj"), NULL, NULL, _T("*.obj|*.*"));
		if (dlg.DoModal() == IDOK)
		{
			std::string s((LPCTSTR)dlg.GetPathName());
			scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
		}
		break;
	}
}

void optionMenu(int id)
{
	if (scene) {
		switch (id)
		{
		case DRAW_NORMALS:
			// Logic to draw normals (turn on)
			scene->setShowNormalsForMeshModels(true);
			break;
		case HIDE_NORMALS:
			// Logic to hide normals (turn off)
			scene->setShowNormalsForMeshModels(false);
			break;
		case DRAW_BOUNDING_BOX:
			// Logic to draw bounding box (turn on)
			scene->setShowBoxForMeshModels(true);
			break;
		case HIDE_BOUNDING_BOX:
			// Logic to hide bounding box (turn off)
			scene->setShowBoxForMeshModels(false);
			break;
		}
	}
	std::cout << "WE GET HERE IN TURNING ON AND OFF THE NORMALS" << std::endl;
	glutPostRedisplay();
}

void mainMenu(int id)
{
	switch (id)
	{
	case MAIN_DEMO:
		scene->drawDemo();
		break;
	case MAIN_ABOUT:
		AfxMessageBox(_T("Computer Graphics"));
		break;
	}
}

void rescaleWindow(bool up_or_down) 
{
	// Your code to rescale the window goes here
	// For example, you might use GLUT functions to reshape the window
	if (up_or_down)
		glutReshapeWindow(2048, 2048);
	else {
		glutReshapeWindow(512, 512);
	}
	glutPostRedisplay();
}

void menuCallback(int menuItem) 
{
	switch (menuItem) {
	case RESCALE_WINDOW_MENU_ITEM_UP:
		// Call a function to rescale the window
		rescaleWindow(true);
		break;
		// Add more cases for additional menu items if needed
	case RESCALE_WINDOW_MENU_ITEM_DOWN:
		rescaleWindow(false);
		break;
	}

}

void initMenu()
{
	int menuFile = glutCreateMenu(fileMenu);
	glutAddMenuEntry("Open..", FILE_OPEN);
	
	// Create the "Normal" submenu
	int optionsSubMenu = glutCreateMenu(optionMenu);
	// Attach the "Normal" submenu to the main menu
	glutAddMenuEntry("Draw Normals", DRAW_NORMALS);
	glutAddMenuEntry("Hide Normals", HIDE_NORMALS);
	glutAddMenuEntry("Draw Bounding Box", DRAW_BOUNDING_BOX);
	glutAddMenuEntry("Hide Bounding Box", HIDE_BOUNDING_BOX);

	int rescaleMenu = glutCreateMenu(menuCallback);
	glutAddMenuEntry("Rescale Window Up", RESCALE_WINDOW_MENU_ITEM_UP);
	glutAddMenuEntry("Rescale Window Down", RESCALE_WINDOW_MENU_ITEM_DOWN);

	glutCreateMenu(mainMenu);
	glutAddSubMenu("File", menuFile);
	glutAddSubMenu("Options", optionsSubMenu);
	glutAddSubMenu("Rescaling Window", rescaleMenu);
	glutAddMenuEntry("Demo", MAIN_DEMO);
	glutAddMenuEntry("About", MAIN_ABOUT);
	
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Attach the menu to a mouse button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------


int my_main(int argc, char** argv)
{
	//----------------------------------------------------------------------------
	// Initialize window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("CG");
	glewExperimental = GL_TRUE;
	glewInit();
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		/*		...*/
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));


	renderer = new Renderer(512, 512);
	scene = new Scene(renderer);
	camera = new Camera();

	std::cout << "start";
	MeshModel* demo_object = new MeshModel("bunny.obj");
	scene->addMeshModel(demo_object);
	std::cout << " end" << std::endl;
	//----------------------------------------------------------------------------

	// Initialize Callbacks

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	initMenu();

	//Init the renderer
	renderer->Init();

	//Set the camera projection we want and send it to renderer (vec3 cast to vec4)
	//camera->LookAt(vec3(1,0,0),vec3(0,0,0),vec3(0,1,0));

	renderer->SetCameraTransformInverse(camera->getTransformInverse());
	renderer->SetProjection(camera->getProjection());


	demo_object->draw(renderer);
	renderer->SwapBuffers();
	glutMainLoop();
	delete scene;
	delete renderer;
	return 0;
}

CWinApp theApp;

using namespace std;

int main(int argc, char** argv)
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		my_main(argc, argv);
	}

	return nRetCode;
}