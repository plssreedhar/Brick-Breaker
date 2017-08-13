#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
#define SPACEBAR 32

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;
typedef struct Point Point;
struct Point
{
    float x;
    float y;
    bool va;
};
struct Node {
 VAO *vv;
 int c1;
 int c2;
 int high;
 int wid;
 float angle;
 float translate;
 float sy;
 int id;
 Point p1;
 Point p2;
};
typedef struct Node Node;
Node laser[100000];
Point inter;
int over=0;int zoom=0;int pan=0;
int score=0;
struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;
int bullet_mode[100000]={0};
Node mir[4];
float speed=0.0055f;
int orientation(Point p, Point q, Point r)
{
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);
    if (val == 0) return 0;  // colinear
    return (val > 0)? 1: 2; // clock or counterclock wise
}
bool onSegment(Point p, Point q, Point r)
{
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
       return true;
 
    return false;
}
Point lineIntersection(Point p1,Point p2,Point p4,Point p5){
float Ax; float Ay;
float Bx; float By;
float Cx; float Cy;
float Dx; float Dy;
float X; float Y;
Point p3;

 Ax=p1.x; Ay=p1.y; Bx=p2.x; By=p2.y;
 Cx=p4.x; Cy=p4.y; Dx=p5.x; Dy=p5.y;
  float  distAB, theCos, theSin, newX, ABpos ;
  if (Ax==Bx && Ay==By || Cx==Dx && Cy==Dy){p3.va=false; return p3;}
  Bx-=Ax; By-=Ay;
  Cx-=Ax; Cy-=Ay;
  Dx-=Ax; Dy-=Ay;
  distAB=sqrt(Bx*Bx+By*By);
  theCos=Bx/(distAB*1.000f);
  theSin=By/(distAB*1.000f);
  newX=Cx*theCos+Cy*theSin;
  Cy  =Cy*theCos-Cx*theSin; Cx=newX;
  newX=Dx*theCos+Dy*theSin;
  Dy  =Dy*theCos-Dx*theSin; Dx=newX;
  if (Cy==Dy){ p3.va=false;return p3;}
  ABpos=Dx+(Cx-Dx)*Dy/((Dy-Cy)*1.000f);
  X=Ax+ABpos*theCos;
  Y=Ay+ABpos*theSin;
  p3.x=X;
  p3.y=Y;
  p3.va=true;
  return p3;
   } 
 float x_intersection,y_intersection;
int intersect_point(Point p1,Point p2,Point p4,Point p5){
  float x0=p1.x, y0=p1.y,x1=p2.x, y1=p2.y;int i;  
    float s1_x, s1_y, s2_x, s2_y, x2=p4.x, y2=p4.y, x3=p5.x, y3=p5.y, q, p, r;
    s1_x = x1 - x0;
    s1_y = y1 - y0;
    s2_x = x3 - x2;     
    s2_y = y3 - y2;
    r=s1_x*s2_y - s2_x*s1_y;
    if(r==0){
      return 0;
    }
    p = (s1_x*(y0-y2) - s1_y*(x0-x2))/(r*1.0f);
    q = (s2_x*(y0-y2) - s2_y*(x0-x2))/(r*1.0f);
    if (p>=0 && p<=1 && q>=0 && q<=1)
    {
        x_intersection = x0 + (q * s1_x);
        y_intersection = y0 + (q * s1_y);
        return 1;
    }
    return 0;
}
bool doIntersect(Point p1, Point q1, Point p2, Point q2)
{
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);
    if (o1 != o2 && o3 != o4)
        return true;
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;if (o2 == 0 && onSegment(p1, q2, q1)) return true;
     if (o3 == 0 && onSegment(p2, p1, q2)) return true;if (o4 == 0 && onSegment(p2, q1, q2)) return true;
 
    return false; 
}
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

void draw3DObject (struct VAO* vao)
{
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    glBindVertexArray (vao->VertexArrayID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float x=1,y=1,z=0;
int keys[12]={0};
Node line;
int bullet_number=-1;float gun_rotation=0.0f;
float translate_bullet[10000]={0};
VAO *createBullet(float x,float y,float h,float w)
{
   VAO *rt;
  // float x1=x+(w/2);float y1=y+(h/2);float x2=x+(w/2);float y2=y-(h/2);float x3=x-(w/2);float y3=y-(h/2);float x4=x-(w/2);float y4=y+(h/2);
  static GLfloat vertex_buffer_data[10];
  vertex_buffer_data[0]=x;vertex_buffer_data[1]=y;vertex_buffer_data[2]=0;
  vertex_buffer_data[3]=x;vertex_buffer_data[4]=y;vertex_buffer_data[5]=0;
  vertex_buffer_data[6]=h;vertex_buffer_data[7]=w;vertex_buffer_data[8]=0;
  static const GLfloat color_buffer_data[]={
    0,0,0, // color 1
    0,0,0, // color 1
    0,0,0, // color 1
  };
   rt = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
   return rt;
}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_M:
                  speed-=0.0010f;
                break;
            case GLFW_KEY_N:
                  speed+=0.0010f;
                break;
            
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_A:
            gun_rotation+=3.0f;
             	break;
            case GLFW_KEY_D:
          z=z+0.3;
             	break;
            case GLFW_KEY_S:
              gun_rotation-=3.0f;
             	break;
            case GLFW_KEY_F:
             	z=z-0.3;
             	break;
            case GLFW_KEY_LEFT:
                if(mods==GLFW_MOD_ALT){
                 x=x-0.3;
                }
                else if(mods==GLFW_MOD_CONTROL){
                  y=y-0.3;
              }break;
            case GLFW_KEY_RIGHT:
                if(mods==GLFW_MOD_ALT){
              x=x+0.3;
                }
                else if(mods==GLFW_MOD_CONTROL){
             y=y+0.3;

                }
                break;
            case SPACEBAR:
                bullet_number++;
                bullet_mode[bullet_number]=1;
                laser[bullet_number].angle=(gun_rotation*(M_PI/180.0));
                laser[bullet_number].vv=createBullet(-3.80f,z,(-3.80f+(0.3f*cos((gun_rotation*(M_PI/180.0))))),(z+(0.3f*sin((gun_rotation*(M_PI/180.0))))));
                break;

            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
	GLfloat fov = 90.0f;
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle,*pad,*gun;//*brick[1000]
Node brick[1000];
// Creates the triangle object used in this sample code
void createTriangle ()
{
  static const GLfloat vertex_buffer_data [] = {
   0,-4,0,
   1,-4,0,
   1,-2,0,

   0,-4,0,
   0,-2,0,
   1,-2,0

      };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };
  triangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createRectangle ()
{
  static const GLfloat vertex_buffer_data [] = {
    0,-4,0,
  -1,-4,0,
   -1,-2,0,

   0,-4,0,
   0,-2,0,
   -1,-2,0
  };

  static const GLfloat color_buffer_data [] = {
   0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1
  };
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
int a;float b;
VAO *der[1000],*desk;
VAO *createCycle()
{
  int a=123;b=90;
  static  GLfloat vertex_buffer_data[19] ;
  vertex_buffer_data[5]=0;vertex_buffer_data[6]=b-4;vertex_buffer_data[7]=3.8;vertex_buffer_data[8]=0;vertex_buffer_data[9]=b-3.9; vertex_buffer_data[10]=4;vertex_buffer_data[11]=0;vertex_buffer_data[12]=b-4;vertex_buffer_data[13]=3.8;vertex_buffer_data[14]=0;
    vertex_buffer_data[15]=b-3.9;vertex_buffer_data[16]=3.8;vertex_buffer_data[17]=0;vertex_buffer_data[0]=b-4;vertex_buffer_data[1]=4;vertex_buffer_data[2]=0;vertex_buffer_data[3]=b-3.9;vertex_buffer_data[4]=4;
        static  GLfloat color_buffer_data[19];

}

VAO *createBrick(float x,float y,float h,float w,float z,int color_id,int counter,int br_flag)
{
    VAO *rt;    float y3=y-(h/2.0f);float x4=x-(w/2.0f);float y4=y+(h/2.0f);
    float x1=x+(w/2.0f);float y1=y+(h/2.0f);float x2=x+(w/2.0f);float y2=y-(h/2.0f);float x3=x-(w/2.0f);
    static GLfloat vertex_buffer_data[19];
    vertex_buffer_data[0]=x1;vertex_buffer_data[1]=y1;vertex_buffer_data[2]=z; vertex_buffer_data[3]=x2;vertex_buffer_data[4]=y2;vertex_buffer_data[5]=z; vertex_buffer_data[6]=x3;vertex_buffer_data[7]=y3;vertex_buffer_data[8]=z; vertex_buffer_data[9]=x3;vertex_buffer_data[10]=y3;vertex_buffer_data[11]=z; vertex_buffer_data[12]=x4;vertex_buffer_data[13]=y4;vertex_buffer_data[14]=z; vertex_buffer_data[15]=x1;vertex_buffer_data[16]=y1;vertex_buffer_data[17]=z;
   
    static  GLfloat color_buffer_data[19];
    if(color_id==1)
    {
        color_buffer_data[0]=1;color_buffer_data[1]=0.2;color_buffer_data[2]=0.2;    color_buffer_data[3]=1;color_buffer_data[4]=0.2;color_buffer_data[5]=0.2 ;  color_buffer_data[6]=1;color_buffer_data[7]=0.2;color_buffer_data[8]=0.2;    color_buffer_data[9]=1;color_buffer_data[10]=0.2;color_buffer_data[11]=0.2;    color_buffer_data[12]=1;color_buffer_data[13]=0.2;color_buffer_data[14]=0.2;    color_buffer_data[15]=1;color_buffer_data[16]=0.2;color_buffer_data[17]=0.2;
    }
    if(color_id==0)
    {
color_buffer_data[9]=0.35;color_buffer_data[10]=0.65;color_buffer_data[11]=0.0;
        color_buffer_data[12]=0.35;color_buffer_data[13]=0.65;color_buffer_data[14]=0.0;   color_buffer_data[15]=0.35;color_buffer_data[16]=0.65;color_buffer_data[17]=0.0;        color_buffer_data[0]=0.35;color_buffer_data[1]=0.65;color_buffer_data[2]=0.0;        color_buffer_data[3]=0.35;color_buffer_data[4]=0.65;color_buffer_data[5]=0.0;        color_buffer_data[6]=0.35;color_buffer_data[7]=0.65;color_buffer_data[8]=0.0;
    }
   if(color_id==1)
    {
        color_buffer_data[0]=1;color_buffer_data[1]=0.2;color_buffer_data[2]=0.2;    color_buffer_data[3]=1;color_buffer_data[4]=0.2;color_buffer_data[5]=0.2 ;  color_buffer_data[6]=1;color_buffer_data[7]=0.2;color_buffer_data[8]=0.2;    color_buffer_data[9]=1;color_buffer_data[10]=0.2;color_buffer_data[11]=0.2;    color_buffer_data[12]=1;color_buffer_data[13]=0.2;color_buffer_data[14]=0.2;    color_buffer_data[15]=1;color_buffer_data[16]=0.2;color_buffer_data[17]=0.2;
    }
    if(color_id==2)
    {
        color_buffer_data[0]=0.0;color_buffer_data[1]=0.0;color_buffer_data[2]=0.0;   color_buffer_data[3]=0.0;color_buffer_data[4]=0.0;color_buffer_data[5]=0.0;   color_buffer_data[6]=0.0;color_buffer_data[7]=0.0;color_buffer_data[8]=0.0;       color_buffer_data[9]=0.0;color_buffer_data[10]=0.0;color_buffer_data[11]=0.0;       color_buffer_data[12]=0.0;color_buffer_data[13]=0.0;color_buffer_data[14]=0.0;   color_buffer_data[15]=0.0;color_buffer_data[16]=0.0;color_buffer_data[17]=0.0;
    } 
    if(br_flag==1){
        brick[counter].c1=x;brick[counter].c2=y;brick[counter].high=h;brick[counter].wid=w;
        brick[counter].id=color_id;
    }
    if(color_id==3)
    {
        color_buffer_data[0]=0.0;color_buffer_data[1]=0.0;color_buffer_data[2]=0.0;   color_buffer_data[3]=0.0;color_buffer_data[4]=0.0;color_buffer_data[5]=0.0;   color_buffer_data[6]=0.0;color_buffer_data[7]=0.0;color_buffer_data[8]=0.0;       color_buffer_data[9]=0.0;color_buffer_data[10]=0.0;color_buffer_data[11]=0.0;       color_buffer_data[12]=0.0;color_buffer_data[13]=0.0;color_buffer_data[14]=0.0;   color_buffer_data[15]=0.0;color_buffer_data[16]=0.0;color_buffer_data[17]=0.0;
    } 
    rt = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    return rt;
}
void createder(int i)
{
	a=rand()%40;
	b=a/5.0;
	static  GLfloat vertex_buffer_data[19] ;
  vertex_buffer_data[5]=0;vertex_buffer_data[6]=b-4;vertex_buffer_data[7]=3.8;vertex_buffer_data[8]=0;vertex_buffer_data[9]=b-3.9; vertex_buffer_data[10]=4;vertex_buffer_data[11]=0;vertex_buffer_data[12]=b-4;vertex_buffer_data[13]=3.8;vertex_buffer_data[14]=0;
    vertex_buffer_data[15]=b-3.9;vertex_buffer_data[16]=3.8;vertex_buffer_data[17]=0;vertex_buffer_data[0]=b-4;vertex_buffer_data[1]=4;vertex_buffer_data[2]=0;vertex_buffer_data[3]=b-3.9;vertex_buffer_data[4]=4;
		/*
   b-4+0.1,4,0,
   b-4,3.8,0,
   b-4,4,0,
   b-4+0.1,3.8,0,
   b-4,3.8,0*/
    static  GLfloat color_buffer_data[19];
	int ass=rand()%3;
  	if(ass==0)
		{
color_buffer_data[0]=0;color_buffer_data[1]=1;color_buffer_data[2]=0;color_buffer_data[3]=0;color_buffer_data[4]=1;color_buffer_data[5]=0;
 color_buffer_data[6]=0;color_buffer_data[7]=1;color_buffer_data[8]=0;color_buffer_data[9]=0;color_buffer_data[10]=1;color_buffer_data[11]=0;
 color_buffer_data[12]=0;color_buffer_data[13]=1;color_buffer_data[14]=0;color_buffer_data[15]=0;color_buffer_data[16]=1;color_buffer_data[17]=0;
}
  else if(ass==1)
  {
color_buffer_data[0]=1;color_buffer_data[1]=0;color_buffer_data[2]=0;color_buffer_data[3]=1;color_buffer_data[4]=0;color_buffer_data[5]=0;
 color_buffer_data[6]=1;color_buffer_data[7]=0;color_buffer_data[8]=0;color_buffer_data[9]=1;color_buffer_data[10]=0;color_buffer_data[11]=0;
 color_buffer_data[12]=1;color_buffer_data[13]=0;color_buffer_data[14]=0;color_buffer_data[15]=1;color_buffer_data[16]=0;color_buffer_data[17]=0;
}
else
{
  color_buffer_data[0]=0;color_buffer_data[1]=0;color_buffer_data[2]=0;color_buffer_data[3]=0;color_buffer_data[4]=0;color_buffer_data[5]=0;
 color_buffer_data[6]=0;color_buffer_data[7]=0;color_buffer_data[8]=0;color_buffer_data[9]=0;color_buffer_data[10]=0;color_buffer_data[11]=0;
 color_buffer_data[12]=0;color_buffer_data[13]=0;color_buffer_data[14]=0;color_buffer_data[15]=0;color_buffer_data[16]=0;color_buffer_data[17]=0;
}
  der[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createPad(){

static GLfloat vertex_buffer_data[19];
vertex_buffer_data[0]=-4;vertex_buffer_data[1]=0.3;vertex_buffer_data[2]=0;vertex_buffer_data[3]=-3.8;vertex_buffer_data[4]=0.3;
vertex_buffer_data[5]=0;vertex_buffer_data[6]=-3.8;vertex_buffer_data[7]=-0.3;vertex_buffer_data[8]=0;vertex_buffer_data[9]=-4;
vertex_buffer_data[10]=0.3;vertex_buffer_data[11]=0;vertex_buffer_data[12]=-4;vertex_buffer_data[13]=-0.3;vertex_buffer_data[14]=0;
vertex_buffer_data[15]=-3.8;vertex_buffer_data[16]=-0.3;vertex_buffer_data[17]=0;

static const GLfloat color_buffer_data [] = {
  
    0,1,1, // color 1
    0,1,1, // color 2
    0,1,1, // color 3

    0,1,1, // color 3
    0,1,1, // color 4
    0,1,1  // color 1
    
  };
  pad=create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createGun()
{

static GLfloat vertex_buffer_data[19];
vertex_buffer_data[0]=0;vertex_buffer_data[1]=0.05;vertex_buffer_data[2]=0;vertex_buffer_data[3]=0.4;vertex_buffer_data[4]=0.05;
vertex_buffer_data[5]=0;vertex_buffer_data[6]=0.4;vertex_buffer_data[7]=-0.05;vertex_buffer_data[8]=0;vertex_buffer_data[9]=0;
vertex_buffer_data[10]=0.05;vertex_buffer_data[11]=0;vertex_buffer_data[12]=0;vertex_buffer_data[13]=-0.05;vertex_buffer_data[14]=0;
vertex_buffer_data[15]=0.4;vertex_buffer_data[16]=-0.05;vertex_buffer_data[17]=0;


static const GLfloat color_buffer_data [] = {

    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1  // color 1
    
  };
    gun=create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
int flag[100000]={0};
float trans[10000]={0};

void createDesk()
{

static GLfloat vertex_buffer_data[19];
vertex_buffer_data[0]=0;vertex_buffer_data[1]=0.05;vertex_buffer_data[2]=0;vertex_buffer_data[3]=0.4;vertex_buffer_data[4]=0.05;
vertex_buffer_data[5]=0;vertex_buffer_data[6]=0.4;vertex_buffer_data[7]=-0.05;vertex_buffer_data[8]=0;vertex_buffer_data[9]=0;
vertex_buffer_data[10]=0.05;vertex_buffer_data[11]=0;vertex_buffer_data[12]=0;vertex_buffer_data[13]=-0.05;vertex_buffer_data[14]=0;
vertex_buffer_data[15]=0.4;vertex_buffer_data[16]=-0.05;vertex_buffer_data[17]=0;
static const GLfloat color_buffer_data [] = {

    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1  // color 1
    
  };
  vertex_buffer_data[0]=0;vertex_buffer_data[1]=0.05;vertex_buffer_data[2]=0;vertex_buffer_data[3]=0.4;vertex_buffer_data[4]=0.05;
vertex_buffer_data[5]=0;vertex_buffer_data[6]=0.4;vertex_buffer_data[7]=-0.05;vertex_buffer_data[8]=0;vertex_buffer_data[9]=0;
vertex_buffer_data[10]=0.05;vertex_buffer_data[11]=0;vertex_buffer_data[12]=0;vertex_buffer_data[13]=-0.05;vertex_buffer_data[14]=0;
vertex_buffer_data[15]=0.4;vertex_buffer_data[16]=-0.05;vertex_buffer_data[17]=0;

    desk=create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
void draw ()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram (programID);
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 1, 0);
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;	// MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateTriangle = glm::translate (glm::vec3(-y, 0, 0)); // glTranslatef
 glm::mat4 triangleTransform = translateTriangle;// * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(triangle);
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(x, 0, 0));        // glTranslatef
  Matrices.model *= (translateRectangle );//* rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE,&MVP[0][0]);
  draw3DObject(rectangle);
 // akatsuki(mir[0].vv,1.0f,3.0f,0.0f,(float)(-45*(M_PI/180.0)),0.0f,0.0f,1.0f);
 // MVP = Projection * View * Model
Matrices.model = glm::mat4(1.0f);
glm::mat4 rotate_box1_re = glm::rotate((float)(-45*(M_PI/180.0)), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
glm::mat4 translate_box1_re = glm::translate (glm::vec3(1,3,0)); // glTranslatef
glm::mat4 box1_rec_Transfor = translate_box1_re*rotate_box1_re ;
  Matrices.model *=box1_rec_Transfor;
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mir[0].vv);
//  akatsuki(mir[1].vv,1.0f,-1.0f,0.0f,(float)(60*(M_PI/180.0)),0.0f,0.0f,1.0f);
// MVP = Projection * View * Model
Matrices.model = glm::mat4(1.0f);
glm::mat4 rotate_box1_rec = glm::rotate((float)(60*(M_PI/180)), glm::vec3(0,0,1));  // rotate about vector (1,0,0)

glm::mat4 translate_box1_rec = glm::translate (glm::vec3(1,-1,0)); // glTranslatef
glm::mat4 box1_rec_Transform = translate_box1_rec*rotate_box1_rec ;
  Matrices.model *=box1_rec_Transform;
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mir[1].vv);
/*for(int i=0;i<1000;i++)
{
	if(flag[i]==1){
  Matrices.model = glm::mat4(1.0f);
glm::mat4 translateBrick = glm::translate (glm::vec3(0, trans[i], 0));        // glTranslatef
 MVP = VP * Matrices.model*translateBrick;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
draw3DObject(der[i]);
trans[i]=trans[i]-0.03;
}
if(bullet_mode[i]==1)
  {
    translate_bullet[i]+=speed;
    // cout<<laser[ew].angle<<endl;
    akatsuki(laser[i].vv,translate_bullet[i]*(cos(laser[i].angle)),translate_bullet[i]*(sin(laser[i].angle)),0.0f,0.0f,0.0f,0.0f,1.0f);
  }
}*/
   int ew=0; 
    int box1_rectangle_key_translate=-y;
    int box2_rectangle_key_translate=x;
    for(ew=0;ew<1000;ew++)
    {
      float coll[2]={0};
      if((brick[ew].c2+brick[ew].translate)<-2.90f && flag[ew]==1)
          {
            float x_cor=brick[ew].c1;
            int tt=brick[ew].id;
            if(((-2.0+box1_rectangle_key_translate)<=x_cor)&&((-1.0+box1_rectangle_key_translate)>=x_cor))coll[0]=1;
            if(((1.0+box2_rectangle_key_translate)<=x_cor)&&((2.0+box2_rectangle_key_translate)>=x_cor))coll[1]=1;
            if((coll[0]==1 && coll[1]==1) && tt!=2)
              score-=1;
            else if(coll[0]==1 && tt == 1){score-=1;flag[ew]=0;}
            else if(coll[0]==1 && tt == 0){score+=1;flag[ew]=0;}
            else if(coll[1]==1 && tt == 1){score+=1;flag[ew]=0;}
            else if(coll[1]==1 && tt == 0){score-=1;flag[ew]=0;}
            else if((coll[1]==1 || coll[0]==1) && tt==2){over=1;}
            else{
            flag[ew]=0;}
            brick[ew].translate=0.0f;
          }
        if(flag[ew]==1)
        {
            brick[ew].translate-=speed;
Matrices.model = glm::mat4(1.0f);
glm::mat4 rotate_box_rec = glm::rotate((float)(0*(M_PI/180)), glm::vec3(0,0,1));  // rotate about vector (1,0,0)

glm::mat4 translate_box_rec = glm::translate (glm::vec3(0,brick[ew].translate,0)); // glTranslatef
glm::mat4 box_rec_Transform = translate_box_rec*rotate_box_rec ;
  Matrices.model *=box_rec_Transform;
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(brick[ew].vv);
        }
        if(bullet_mode[ew]==1)
        {
            translate_bullet[ew]+=0.1f;
            float rtt=laser[ew].translate;
        laser[ew].c1=laser[ew].sy+((float)(0.30f+translate_bullet[ew])*(float)cos(laser[ew].angle));
        laser[ew].c2=rtt+((0.30f+translate_bullet[ew])*(float)sin(laser[ew].angle));
        laser[ew].p2.x=laser[ew].c1;
        laser[ew].p2.y=laser[ew].c2;
        laser[ew].p1.x=laser[ew].sy+((float)(translate_bullet[ew])*(float)cos(laser[ew].angle));
        laser[ew].p1.y=rtt+((translate_bullet[ew])*(float)sin(laser[ew].angle));
Matrices.model = glm::mat4(1.0f);
glm::mat4 rotate_box_re = glm::rotate((float)0.0, glm::vec3(0,0,1));  // rotate about vector (1,0,0)
glm::mat4 translate_box_re = glm::translate (glm::vec3(translate_bullet[ew]*(cos(laser[ew].angle)),translate_bullet[ew]*(sin(laser[ew].angle)),0.0f)); // glTranslatef
glm::mat4 box_re_Transform = translate_box_re*rotate_box_re ;
  Matrices.model *=box_re_Transform;
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(laser[ew].vv);
        }
    }
    for(ew=0;ew<1000;ew++)
    {
        int eww;
        for(eww=0;eww<50;eww++)
        {
          if(flag[ew]==1 && bullet_mode[eww]==1)
          {
            float z1=laser[eww].c1;float z2=laser[eww].c2;float z3=brick[ew].c1-0.1650f;float z4=brick[ew].c1+0.1650f;
            float z5=brick[ew].c2+brick[ew].translate-0.1250f;float z6=brick[ew].c2+brick[ew].translate+0.1250f; 
            if(((z3<=z1)&&(z4>=z1))&&((z5<z2)&&(z6>z2)))
            {
              if(brick[ew].id==2){
              bullet_mode[eww]=0;
              flag[ew]=0;
              score+=1;}
              else
                score-=1;
            }
          }
        }
    }
    for(ew=0;ew<1000;ew++)
    {
      if(bullet_mode[ew]==1 )
      {
          if(intersect_point(mir[0].p1, mir[0].p2, laser[ew].p1, laser[ew].p2)==1){
           laser[ew].angle=(float)(-90.0f*(M_PI/180.0))-laser[ew].angle;
            laser[ew].translate=y_intersection+(0.3f*sin(laser[ew].angle));
            laser[ew].sy=x_intersection+(0.3f*cos(laser[ew].angle));
            translate_bullet[ew]=0.0f;
            laser[ew].c1=laser[ew].sy+((float)(0.30f+translate_bullet[ew])*(float)cos(laser[ew].angle));
        laser[ew].c2=laser[ew].translate+((0.30f+translate_bullet[ew])*(float)sin(laser[ew].angle));
        laser[ew].p2.x=laser[ew].c1;
        laser[ew].p2.y=laser[ew].c2;
        laser[ew].p1.x=laser[ew].sy+((float)(translate_bullet[ew])*(float)cos(laser[ew].angle));
        laser[ew].p1.y=laser[ew].translate+((translate_bullet[ew])*(float)sin(laser[ew].angle));
        }
          if(intersect_point(mir[1].p1, mir[1].p2, laser[ew].p1, laser[ew].p2)==1){
            laser[ew].angle=(float)(120.0f*(M_PI/180.0))-laser[ew].angle;
            laser[ew].translate=y_intersection+0.3f*sin(laser[ew].angle);
            laser[ew].sy=x_intersection+0.3f*cos(laser[ew].angle);
            
            translate_bullet[ew]=0.0f;
            laser[ew].c1=laser[ew].sy+((float)(0.30f+translate_bullet[ew])*(float)cos(laser[ew].angle));
        laser[ew].c2=laser[ew].translate+((0.30f+translate_bullet[ew])*(float)sin(laser[ew].angle));
        laser[ew].p2.x=laser[ew].c1;
        laser[ew].p2.y=laser[ew].c2;
        laser[ew].p1.x=laser[ew].sy+((float)(translate_bullet[ew])*(float)cos(laser[ew].angle));
        laser[ew].p1.y=laser[ew].translate+((translate_bullet[ew])*(float)sin(laser[ew].angle));
        }
      }
    }
Matrices.model = glm::mat4(1.0f);
glm::mat4 translatePad = glm::translate (glm::vec3(0, z, 0)); 
MVP= VP * Matrices.model*translatePad;
   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
draw3DObject(pad);
Matrices.model = glm::mat4(1.0f);
glm::mat4 translateGun = glm::translate (glm::vec3(-4, z, 0));
glm::mat4 rotateGun = glm::rotate((float)(gun_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
MVP= VP * Matrices.model*translateGun*rotateGun;
   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
draw3DObject(gun);
}

GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    return window;
}
void initGL (GLFWwindow* window, int width, int height)
{
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
	createGun();createPad();
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");	
	reshapeWindow (window, width, height);
	glClearColor (1.0f, 1.0f, 1.0f, 0.5f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;
    mir[0].vv=createBrick(0.0f,0.0f,0.1f,0.75,1.0,3,1,0);
    mir[1].vv=createBrick(0.0f,0.0f,0.1f,0.75,1.0,3,1,0); 

    mir[0].p1.x=1.0f+(0.375f*cos((float)(-45*(M_PI/180.0))));
    mir[0].p1.y=3.0f+(0.375f*sin((float)(-45*(M_PI/180.0))));

    mir[1].p1.x=1.0f+(0.375f*cos((float)(60*(M_PI/180.0))));
    mir[1].p1.y=-1.0f+(0.375f*sin((float)(60*(M_PI/180.0))));

    mir[0].p2.x=1.0f-(0.375f*cos((float)(-45*(M_PI/180.0))));
    mir[0].p2.y=3.0f-(0.375f*sin((float)(-45*(M_PI/180.0))));

    mir[1].p2.x=1.0f-(0.375f*cos((float)(60*(M_PI/180.0))));
    mir[1].p2.y=-1.0f-(0.375f*sin((float)(60*(M_PI/180.0))));

int cor=-1;int j;
for(j=0;j<52;j++)
        brick[j].translate=0.0f;
    int counter=-1;
    int ll=0;
    while (!glfwWindowShouldClose(window)) {
        draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 2.5) { // atleast 0.5s elapsed since last frame
       //  cor++;
                //  cout <<cor<<endl;
      //   flag[cor]=1;
           //               createder(cor);
                           counter++;flag[counter]=1;
            float sign;
            cout<<"Score is :"<<score<<endl;
            srand(time(NULL));
            if(rand()%2==1)sign=-1.0;
                        if(counter>50){counter=-1;ll=1;}

            else sign=1.0;
            srand(time(NULL));
            int brick_color=rand()%3;
            if(ll==0)
            {
              float checkk=(sign*(rand()%18)/5.0f);
          
           brick[counter].vv=createBrick(checkk,3.750f,0.250f,0.330f,2.0,brick_color,counter,1);
            }
            last_update_time = current_time;
            if(counter>50){counter=-1;ll=1;}
        }
        if(over==1)
    {
      exit(0);
      quit(window);
    }
  }
    glfwTerminate();
}
