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
#include "CPopup.h"
#include "CColorPicker.h"

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))


enum MENU_STATES {
	OPEN_FILE_OBJ,
	ADD_CAMERA_ORTHO,
	ADD_CAMERA_PROJECTION,
	MAIN_DEMO,
	MAIN_ABOUT,

	DELETE_MESH,
	DELETE_CAMERA,

	RESCALE_WINDOW_MENU_ITEM_UP,
	RESCALE_WINDOW_MENU_ITEM_DOWN,

	CHANGE_INCREMENT,

	DRAW_NORMALS,
	HIDE_NORMALS,
	DRAW_VERTEX_NORMALS,
	HIDE_VERTEX_NORMALS,
	DRAW_BOUNDING_BOX,
	HIDE_BOUNDING_BOX,

	ADD_TETRAHEDRON,
	ADD_CUBE
};

Scene* scene;
Renderer* renderer;
float m_time;

int last_x,last_y;
bool lb_down,rb_down,mb_down;

float increment = 0.2;

//----------------------------------------------------------------------------
// Camera + Scene modiications
//----------------------------------------------------------------------------

void swapCameras(){
	scene->cycleActiveCamera();
	renderer->setCameraMatrixes(scene->getActiveCamera());
	glutPostRedisplay();
}

#define TRY_FLOAT(var, text) try { var = std::stof(text); } catch (const std::invalid_argument& e) {std::cout<<"BAD_INPUT"<<std::endl;return;} catch (const std::out_of_range& e) {return;}

void changeIncrement(){
	renderer->FillEdges(0.1, vec3(0.9, 0.1, 0.1));
	display();
	std::string userInput;
	
	std::cout << "Set increment (default=0.2): ";
	std::cin >> userInput;
	TRY_FLOAT(increment, userInput);
}

void addProjCamera(){
	
	CPopup c;
	int result = c.DoModal();
	if(result == IDCANCEL){
		return;
	}
	
	Camera* camera = new Camera();
	float fov_degrees = 70;
	float aspect_ratio = 1;
	float zNear = 0.5;
	float zFar = 5;
	if(result == IDOK){
		fov_degrees = c.m_sliderval;
		try{
			aspect_ratio = _ttof(c.m_msg1);
			zNear = _ttof(c.m_msg2);
			zFar = _ttof(c.m_msg3);
		}
		catch(exception e){
			aspect_ratio = 1;
			zNear = 0.5;
			zFar = 5;
		}
	}
	
	fov_degrees = Radians(fov_degrees);
	camera->LookAt(vec3(1,1,1),vec3(-1,0,0),vec3(0,1,0));
	//TEMP ORTHOGRAPHIC
	camera->Perspective(fov_degrees,aspect_ratio,zNear,zFar);
	scene->addCamera(camera);
	glutPostRedisplay();
}

void addOrthoCamera(){
	CPopupOrtho c;
	int result = c.DoModal();
	if(result == IDCANCEL){
		return;
	}
	float width = 2;
	float height = 2;
	float zNear = 0.5;
	float zFar = 5;
	if(result == IDOK){
		try{
			width = _ttof(c.m_msg1);
			height = _ttof(c.m_msg2);
			zNear = _ttof(c.m_msg3);
			zFar = _ttof(c.m_msg4);
		}
		catch(exception e){

		}
	}
	Camera* camera = new Camera();

	camera->LookAt(vec3(1,0,1),vec3(-1,0,0),vec3(0,1,0));
	camera->Ortho(-width/2,width/2,-height/2,height/2,zNear,zFar);
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

void changeLight(){
	/*CColorDialog c;
	if(c.DoModal() != IDOK){
		return;
	}*/
	if(scene->getMovingModel()){
		CColorPicker colordialog(nullptr,&renderer->getAmbientLight());
		if(colordialog.DoModal() != IDOK){
			return;
		}
		float intensity = colordialog.m_sliderval/100.f;
		COLORREF color = colordialog.m_color.GetColor();
		
		vec3 colorvec = vec3((float)GetRValue(color)/255.f,
		(float)GetGValue(color)/255.f,
		(float)GetBValue(color)/255.f);
		renderer->setAmbientLight(AmbientLight(intensity,colorvec));
	}
	else{
		Light* light = scene->getSelectedLight();
		CColorPicker colordialog(nullptr,light);
		if(colordialog.DoModal() != IDOK){
			return;
		}
		float intensity = colordialog.m_sliderval/100.f;
		COLORREF color = colordialog.m_color.GetColor();
		
		vec3 colorvec = vec3((float)GetRValue(color)/255.f,
		(float)GetGValue(color)/255.f,
		(float)GetBValue(color)/255.f);
		light->setColor(colorvec);
		light->setIntensity(intensity);
	}
}
//----------------------------------------------------------------------------
// Callbacks
//----------------------------------------------------------------------------

void display( void ){
	//if false, moving lights
	if(!scene->getMovingModel()){
		renderer->FillEdges(0.05, vec3(0.5,0.5,0.2));
	}
	if(scene->getWorldControl()){
		renderer->FillEdges(0.02, vec3(0.1, 0.1, 0.5));
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
	renderer->setCameraMatrixes(scene->getActiveCamera());
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
		renderer->setCameraMatrixes(scene->getActiveCamera());
		break;
	case 'z': //return model to center
		scene->returnModelToCenter();
		break;
	case '.':
		scene->getActiveCamera()->translate(0, 0, 0.1, scene->getWorldControl());
		renderer->setCameraMatrixes(scene->getActiveCamera());
		break;
	case 'v':
		scene->rotateCameraToSelectedObject();
		renderer->setCameraMatrixes(scene->getActiveCamera());
		break;
	case 'm':
		scene->getActiveCamera()->rotate(-15, 1, scene->getWorldControl());
		renderer->setCameraMatrixes(scene->getActiveCamera());
		break;
	case 'n':
		scene->getActiveCamera()->rotate(15, 1, scene->getWorldControl());
		renderer->setCameraMatrixes(scene->getActiveCamera());
		break;
	case 't':
		scene->scaleObject(1.3f); // Increase scale by 30%
		break;
	case 'r':
		scene->scaleObject(0.77f); // Decrease scale by 30%
		break;
	case 'a':
		scene->translateObject(-increment, 0, 0);
		break;
	case 'd':
		scene->translateObject(increment, 0, 0);
		break;
	case 'w':
		scene->translateObject(0, 0, -increment);
		break;
	case 's':
		scene->translateObject(0, 0, increment);
		break;
	case 'e':
		scene->translateObject(0, increment, 0);
		break;
	case 'q':
		scene->translateObject(0, -increment, 0);
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
	case 'g':
		scene->toggleMovingModel();
		break;
	case ' ':
		swapCameras();
		break;
	case '1':
		scene->setFillObj(!(scene->getFillObj()));
		break;
	case '2':
		scene->changeCurrsColor();
		break;
	case '3':
		scene->changeShadingMethod();
		break;
	case '4':
		renderer->setFogFlag(!(renderer->getFogFlag()));
		break;
	case 'h':
		changeLight();
		break;
	case '6':
		renderer->setAntiAliasing(!(renderer->getAntiAliasingFlag()));
		break;
	default:
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

void deleteMenu(int id){
	switch(id){
		case DELETE_MESH:
			scene->removeSelectedObject();
			break;
		case DELETE_CAMERA:
			scene->removeSelectedCamera();
			renderer->setCameraMatrixes(scene->getActiveCamera());
			break;
	}
	glutPostRedisplay();
}

void primMenu(int id) {
	PrimMeshModel* model;
	switch(id){
		case ADD_CUBE:
			model = new PrimMeshModel(PRIM_CUBE);
			break;
		case ADD_TETRAHEDRON:
			model = new PrimMeshModel(PRIM_TETRAHEDRON);
			break;
		default:
			return;
	}
	scene->addMeshModel(model);
	glutPostRedisplay();
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
		case DRAW_VERTEX_NORMALS:
			// Logic to draw normals to vertices (turn on)
			scene->setShowNormalsToVerticesForMeshModels(true);
			break;
		case HIDE_VERTEX_NORMALS:
			// Logic to draw normals to vertices (turn off)
			scene->setShowNormalsToVerticesForMeshModels(false);
			break;
		}
	}
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
		AfxMessageBox(_T("Controls:\n\nTab - Cycle selected Mesh\nWASDQE - Move the selected mesh.\nJKLI - Rotate selected mesh\nF - Toggle world/model space controls\n\nSpace - Cycle active camera\n[Arrows , .] - Move camera\n\nBlue outline means World space\nRed outline means an input needs to be put"));
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
	case CHANGE_INCREMENT:
		changeIncrement();
		break;
	}

}

void initMenu()
{
	int primitivesMenu = glutCreateMenu(primMenu);
	glutAddMenuEntry("Tetrahedron", ADD_TETRAHEDRON);
	glutAddMenuEntry("Cube", ADD_CUBE);

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
	glutAddMenuEntry("Draw Normals To Vertices", DRAW_VERTEX_NORMALS);
	glutAddMenuEntry("Hide Normals To Vertices", HIDE_VERTEX_NORMALS);
	glutAddMenuEntry("Draw Bounding Box", DRAW_BOUNDING_BOX);
	glutAddMenuEntry("Hide Bounding Box", HIDE_BOUNDING_BOX);
	//Draw Hide cameras
	//Draw hide vertex normals

	int rescaleMenu = glutCreateMenu(menuCallback);
	glutAddMenuEntry("Rescale Window Up", RESCALE_WINDOW_MENU_ITEM_UP);
	glutAddMenuEntry("Rescale Window Down", RESCALE_WINDOW_MENU_ITEM_DOWN);
	
	glutAddMenuEntry("Change increment", CHANGE_INCREMENT);

	int deleteSubMenu = glutCreateMenu(deleteMenu);
	glutAddMenuEntry("Delete Sel. Mesh", DELETE_MESH);
	glutAddMenuEntry("Delete Sel. Camera", DELETE_CAMERA);

	glutCreateMenu(mainMenu);
	glutAddSubMenu("New", menuFile);
	glutAddSubMenu("Delete", deleteSubMenu);
	glutAddSubMenu("View", optionsSubMenu);
	glutAddSubMenu("Window", rescaleMenu);
	glutAddMenuEntry("Demo", MAIN_DEMO);
	glutAddMenuEntry("Help", MAIN_ABOUT);
	
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
	glutCreateWindow("Wireframe render - RightClick for options");
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
	Light* light = new PointLight(1,vec3(1,1,1),vec3(-2,0,1));
	Light* light2 = new DirectionalLight(1,vec3(1,0,1),vec3(0,1,0));
	renderer->setAmbientLight(AmbientLight(1,vec3(0.3,0.3,0.3)));
	Fog* fog = new Fog();

	std::cout << "[ ] Camera transform: " << std::endl;
	camera->LookAt(vec3(0,0,1),vec3(0,0,-1),vec3(0,1,0));
	camera->Ortho(-1,1,-1,1,0,5);
	scene->addCamera(camera);
	std::cout <<"!"<< camera->getProjection();
	renderer->setCameraMatrixes(scene->getActiveCamera());

	scene->addLightSource(light);
	scene->addLightSource(light2);

	scene->addFog(fog);

	std::cout << "[ ] Reading mesh files... ";
	MeshModel* demo_object = new MeshModel("meshes/bunny.obj");
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
