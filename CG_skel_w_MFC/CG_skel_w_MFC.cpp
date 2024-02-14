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


enum MENU_STATES {
	OPEN_FILE_OBJ,
	ADD_CAMERA_ORTHO,
	ADD_CAMERA_PROJECTION,
	MAIN_DEMO,
	MAIN_ABOUT,

	RESCALE_WINDOW_MENU_ITEM_UP,
	RESCALE_WINDOW_MENU_ITEM_DOWN,

	DRAW_NORMALS,
	HIDE_NORMALS,
	DRAW_BOUNDING_BOX,
	HIDE_BOUNDING_BOX,

	PYRAMID,
	CUBE
};

Scene* scene;
Renderer* renderer;
float m_time;

int last_x,last_y;
bool lb_down,rb_down,mb_down;

//----------------------------------------------------------------------------
// Camera + Scene modiications
//----------------------------------------------------------------------------

void swapCameras(){
	scene->cycleActiveCamera();
	renderer->setCameraMatrixes(scene->getActiveCamera()->getTransformInverse(),scene->getActiveCamera()->getProjection());
	glutPostRedisplay();
}

#define TRY_FLOAT(var, text) try { var = std::stof(text); } catch (const std::invalid_argument& e) {return;} catch (const std::out_of_range& e) {return;}
void addProjCamera(){
	int result = AfxMessageBox(_T("Enter Projection data in the TERMINAL.\npress CANCEL if you do not wish to continue."), MB_ICONINFORMATION | MB_OKCANCEL);
	if(result == IDCANCEL){
		return;
	}

	Camera* camera = new Camera();

	std::cout << "enter aspect ratio: ";
	std::string userInput;
	std::cin >> userInput;
	float aspect_ratio = 1;
	TRY_FLOAT(aspect_ratio, userInput);
	
	
	camera->LookAt(vec3(1,1,1),vec3(-1,0,0),vec3(0,1,0));
	//TEMP ORTHOGRAPHIC
	camera->Perspective(1,1,0.5,4);
	scene->addCamera(camera);
	glutPostRedisplay();
}

void addOrthoCamera(){

	int result = AfxMessageBox(_T("You will be sent to the CMD to input the Orthographic specs.\npress CANCEL if you do not wish to continue."), MB_ICONINFORMATION | MB_OKCANCEL);
	if(result == IDCANCEL){
		return;
	}

	renderer->FillEdges(0.05,0.9,0.1,0.1);
	glutPostRedisplay();
	Camera* camera = new Camera();

	std::cout << "enter aspect ratio: ";
	std::string userInput;
	std::cin >> userInput;
	float aspect_ratio = 1;
	try {
        // Try to convert the input string to a float
        aspect_ratio = std::stof(userInput);
        
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
		return;
    } catch (const std::out_of_range& e) {
        std::cerr << "Out of range: " << e.what() << std::endl;
        return;
    }

	//std::cout << "enter z-min: ";
	//std::string userInput2;
	//std::cin >> userInput2;
	
	camera->LookAt(vec3(1,1,1),vec3(-1,0,0),vec3(0,1,0));
	camera->Ortho(-1,1,-1,1,-0.5,-5);
	scene->addCamera(camera);
	glutPostRedisplay();
}

void readFromFile(){
	CFileDialog dlg(TRUE, _T(".obj"), NULL, NULL, _T("*.obj|*.*"));
	if (dlg.DoModal() == IDOK)
	{
		std::string s((LPCTSTR)dlg.GetPathName());
		scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
		glutPostRedisplay();
	}
}
//----------------------------------------------------------------------------
// Callbacks
//----------------------------------------------------------------------------

void display( void ){
	if(scene->getWorldControl()){
		renderer->FillEdges(0.02,0.1,0.1,0.5);
	}
	scene->draw();
	renderer->SwapBuffers();
}

void reshape( int width, int height )
{
	// Update the renderer's buffers
	renderer->ResizeBuffers(width, height);

	// Set the viewport to the entire window
	glViewport(0, 0, width, height);

	// Update the camera's projection matrix
	//float aspect_ratio = static_cast<float>(width) / height;
	//camera->UpdateProjectionMatrix(aspect_ratio);

	// Redraw the scene
	glutPostRedisplay();
}

void keyboard_special( int key, int x, int y ){
	switch (key) {
		case GLUT_KEY_LEFT:
			scene->getActiveCamera()->translate(-0.1, 0, 0, scene->getWorldControl());
			break;
		case GLUT_KEY_RIGHT:
			scene->getActiveCamera()->translate(0.1, 0, 0, scene->getWorldControl());
			break;
		case GLUT_KEY_UP:
			scene->getActiveCamera()->translate(0, 0.1, 0, scene->getWorldControl());
			break;
		case GLUT_KEY_DOWN:
			scene->getActiveCamera()->translate(0, -0.1, 0, scene->getWorldControl());
			break;
		default:
			//fail
			return;
	}
	
	//if key was accepted
	renderer->setCameraMatrixes(scene->getActiveCamera()->getTransformInverse(),scene->getActiveCamera()->getProjection());
	glutPostRedisplay();
}

void keyboard( unsigned char key, int x, int y )
{
	switch ( key ) {
	case 033:
		exit( EXIT_SUCCESS );
		break;
	case 9:
		scene->cycleSelectedObject();
		break;
	case ',':
		scene->getActiveCamera()->translate(0, 0, -0.1, scene->getWorldControl());
		renderer->setCameraMatrixes(scene->getActiveCamera()->getTransformInverse(),scene->getActiveCamera()->getProjection());
		break;
	case '.':
		scene->getActiveCamera()->translate(0, 0, 0.1, scene->getWorldControl());
		renderer->setCameraMatrixes(scene->getActiveCamera()->getTransformInverse(),scene->getActiveCamera()->getProjection());
		break;
	case 't':
		scene->scaleObject(1.3f); // Increase scale by 30%
		break;
	case 'r':
		scene->scaleObject(0.77f); // Decrease scale by 30%
		break;
	case 'a':
		scene->translateObject(-0.2, 0, 0);
		break;
	case 'd':
		scene->translateObject(0.2, 0, 0);
		break;
	case 'w':
		scene->translateObject(0, 0, -0.2);
		break;
	case 's':
		scene->translateObject(0, 0, 0.2);
		break;
	case 'e':
		scene->translateObject(0, 0.2, 0);
		break;
	case 'q':
		scene->translateObject(0, -0.2, 0);
		break;
	case 'j':
		scene->rotateObject(-30, 1);
		break;
	case 'l':
		scene->rotateObject(30, 1);
		break;
	case 'k':
		scene->rotateObject(30, 0);
		break;
	case 'i':
		scene->rotateObject(-30, 0);
		break;
	case 'f':
		scene->setWorldControl(!scene->getWorldControl());
		break;
	case ' ':
		swapCameras();
		break;
	default:
		//fail
		return;
	}

	//if key was accepted
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	//button = {GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON}
	//state = {GLUT_DOWN,GLUT_UP}
	
	//set down flags
	switch(button) {
		case GLUT_LEFT_BUTTON:
			lb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_RIGHT_BUTTON:
			rb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_MIDDLE_BUTTON:
			mb_down = (state==GLUT_UP)?0:1;	
			break;
	}

	// add your code
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx=x-last_x;
	int dy=y-last_y;
	// update last x,y
	last_x=x;
	last_y=y;
}

//----------------------------------------------------------------------------
// Menus
//----------------------------------------------------------------------------

void primMenu(int id){

}

void fileMenu(int id)
{
	switch (id)
	{
	case OPEN_FILE_OBJ:
		readFromFile();
		break;
	case ADD_CAMERA_ORTHO:
		addOrthoCamera();
		break;
	case ADD_CAMERA_PROJECTION:
		addProjCamera();
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
	
	int newWidth, newHeight;

	if (up_or_down) {
		newWidth = 2048;
		newHeight = 2048;
	}
	else {
		newWidth = 512;
		newHeight = 512;
	}

	if (newWidth == glutGet(GLUT_WINDOW_WIDTH) && newHeight == glutGet(GLUT_WINDOW_HEIGHT)) {
		return;
	}
	//scene->translateObject((newWidth - glutGet(GLUT_WINDOW_WIDTH)) / 2, (newHeight - glutGet(GLUT_WINDOW_HEIGHT)) / 2, 0);

	glutReshapeWindow(newWidth, newHeight);
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
	int primitivesMenu = glutCreateMenu(primMenu);
	glutAddMenuEntry("Pyramid", PYRAMID);
	glutAddMenuEntry("Cube", CUBE);

	int menuFile = glutCreateMenu(fileMenu);
	glutAddMenuEntry("Orthographic Camera", ADD_CAMERA_ORTHO);
	glutAddMenuEntry("Perspective Camera", ADD_CAMERA_PROJECTION);
	glutAddMenuEntry(".OBJ Mesh...", OPEN_FILE_OBJ);
	glutAddSubMenu("Primitives", primitivesMenu);
	
	// Create the "Normal" submenu
	int optionsSubMenu = glutCreateMenu(optionMenu);
	// Attach the "Normal" submenu to the main menu
	glutAddMenuEntry("Draw Normals", DRAW_NORMALS);
	glutAddMenuEntry("Hide Normals", HIDE_NORMALS);
	glutAddMenuEntry("Draw Bounding Box", DRAW_BOUNDING_BOX);
	glutAddMenuEntry("Hide Bounding Box", HIDE_BOUNDING_BOX);
	//Draw Hide cameras
	//Draw hide vertex normals

	int rescaleMenu = glutCreateMenu(menuCallback);
	glutAddMenuEntry("Rescale Window Up", RESCALE_WINDOW_MENU_ITEM_UP);
	glutAddMenuEntry("Rescale Window Down", RESCALE_WINDOW_MENU_ITEM_DOWN);

	glutCreateMenu(mainMenu);
	glutAddSubMenu("New", menuFile);
	glutAddSubMenu("View", optionsSubMenu);
	glutAddSubMenu("Window", rescaleMenu);
	glutAddMenuEntry("Demo", MAIN_DEMO);
	glutAddMenuEntry("About", MAIN_ABOUT);
	
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Attach the menu to a mouse button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Main
//----------------------------------------------------------------------------

int my_main(int argc, char** argv)
{
	//----------------------------------------------------------------------------
	// Initialize window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(1024, 1024);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("CG - press r/t to rescale, a,d,w,s to rotate");
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

	renderer = new Renderer(1024, 1024);
	scene = new Scene(renderer);
	Camera* camera = new Camera();

	std::cout << "[ ] Camera transform: " << std::endl;
	camera->LookAt(vec3(1,1,1),vec3(0,0,-1),vec3(0,1,0));
	camera->Ortho(-1,1,-1,1,0,5);
	scene->addCamera(camera);
	std::cout <<"!"<< camera->getProjection();
	renderer->setCameraMatrixes(scene->getActiveCamera()->getTransformInverse(),scene->getActiveCamera()->getProjection());

	std::cout << "[ ] Reading mesh files... ";
	MeshModel* demo_object = new MeshModel("meshes/fox.obj");
	scene->addMeshModel(demo_object);
	std::cout << " Done!" << std::endl;
	//----------------------------------------------------------------------------

	// Initialize Callbacks

	glutDisplayFunc( display );
	glutSpecialFunc( keyboard_special );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	initMenu();
	
	//Init the renderer
	renderer->Init();

	std::cout << scene->getWorldControl() << " : (#)" << std::endl;
	
	//Set the camera projection we want and send it to renderer (vec3 cast to vec4)

	std::cout << "[V] Done with the initialization! " << std::endl;
	glutMainLoop();
	delete scene;
	delete renderer;
	return 0;
}

CWinApp theApp;

using namespace std;

int main( int argc, char **argv )
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
		my_main(argc, argv );
	}
	
	return nRetCode;
}
