#ifndef CHESS_WITH_GRAPHICS
#define CHESS_WITH_GRAPHICS


#include <stdio.h>
#include <random>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using std::cin;
using std::cerr;

using std::cout;
using std::flush;
using std::endl;


#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#include "glm/glm.hpp" //general vector types
#include "glm/gtc/matrix_transform.hpp" // for glm::ortho
#include "glm/gtc/type_ptr.hpp" //to send matricies gpu-side
#include "glm/gtx/transform.hpp"


#define GL_GLEXT_PROTOTYPES

#include "LodePNG/lodepng.h"
// Good, simple png library

#include "perlin.h"
//perlin noise generation


#include "game.h"
//this is our chess game code


#include <SDL.h>
#include <SDL_opengl.h>





//DEBUG STUFF

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{

  bool show_high_severity         = true;
  bool show_medium_severity       = true;
  bool show_low_severity          = true;
  bool show_notification_severity = true;

  if(severity == GL_DEBUG_SEVERITY_HIGH && show_high_severity)
    fprintf( stderr, "        GL CALLBACK: %s type = 0x%x, severity = GL_DEBUG_SEVERITY_HIGH, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, message );

  if(severity == GL_DEBUG_SEVERITY_MEDIUM && show_medium_severity)
    fprintf( stderr, "        GL CALLBACK: %s type = 0x%x, severity = GL_DEBUG_SEVERITY_MEDIUM, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, message );

  if(severity == GL_DEBUG_SEVERITY_LOW && show_low_severity)
    fprintf( stderr, "        GL CALLBACK: %s type = 0x%x, severity = GL_DEBUG_SEVERITY_LOW, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, message );

  if(severity == GL_DEBUG_SEVERITY_NOTIFICATION && show_notification_severity)
    fprintf( stderr, "        GL CALLBACK: %s type = 0x%x, severity = GL_DEBUG_SEVERITY_NOTIFICATION, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, message );

  // SDL_Delay(1000);

}








//  ╔═╗┬ ┬┌─┐┌┬┐┌─┐┬─┐
//  ╚═╗├─┤├─┤ ││├┤ ├┬┘
//  ╚═╝┴ ┴┴ ┴─┴┘└─┘┴└─

class Shader
{
  public:
    GLuint Program;
    // Constructor generates the shader on the fly
    Shader( const GLchar *vertexPath, const GLchar *fragmentPath, bool verbose=false)
    {

        // 1. Retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensures ifstream objects can throw exceptions:
        vShaderFile.exceptions ( std::ifstream::badbit );
        fShaderFile.exceptions ( std::ifstream::badbit );
        try
        {
            // Open files
            vShaderFile.open( vertexPath );
            fShaderFile.open( fragmentPath );
            std::stringstream vShaderStream, fShaderStream;
            // Read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf( );
            fShaderStream << fShaderFile.rdbuf( );
            // close file handlers
            vShaderFile.close( );
            fShaderFile.close( );
            // Convert stream into string
            vertexCode = vShaderStream.str( );
            fragmentCode = fShaderStream.str( );
        }
        catch ( std::ifstream::failure &e )
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }


        const GLchar *vShaderCode = vertexCode.c_str( );
        const GLchar *fShaderCode = fragmentCode.c_str( );
        // 2. Compile shaders
        GLuint vertex, fragment;
        GLint success;
        GLchar infoLog[512];


        // Vertex Shader
        vertex = glCreateShader( GL_VERTEX_SHADER );

        glShaderSource( vertex, 1, &vShaderCode, NULL );

        glCompileShader( vertex );



        // Print compile errors if any
        glGetShaderiv( vertex, GL_COMPILE_STATUS, &success );
        if ( !success )
        {
            glGetShaderInfoLog( vertex, 512, NULL, infoLog );
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // Fragment Shader
        fragment = glCreateShader( GL_FRAGMENT_SHADER );
        glShaderSource( fragment, 1, &fShaderCode, NULL );
        glCompileShader( fragment );


        // Print compile errors if any
        glGetShaderiv( fragment, GL_COMPILE_STATUS, &success );
        if ( !success )
        {
            glGetShaderInfoLog( fragment, 512, NULL, infoLog );
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // Shader Program
        this->Program = glCreateProgram( );
        glAttachShader( this->Program, vertex );
        glAttachShader( this->Program, fragment );
        glLinkProgram( this->Program );
        // Print linking errors if any
        glGetProgramiv( this->Program, GL_LINK_STATUS, &success );
        if (!success)
        {
            glGetProgramInfoLog( this->Program, 512, NULL, infoLog );
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        // Delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader( vertex );
        glDeleteShader( fragment );

    }
    // Uses the current shader
    void Use( )
    {
        glUseProgram( this->Program );
    }
};




//  ╔═╗┌─┐┌─┐┌┐┌┌─┐┬    ╔═╗┌─┐┌┐┌┌┬┐┌─┐┬┌┐┌┌─┐┬─┐
//  ║ ║├─┘├┤ ││││ ┬│    ║  │ ││││ │ ├─┤││││├┤ ├┬┘
//  ╚═╝┴  └─┘┘└┘└─┘┴─┘  ╚═╝└─┘┘└┘ ┴ ┴ ┴┴┘└┘└─┘┴└─

class opengl_container
{
public:
  opengl_container();
  ~opengl_container();

  // void update_rotation();
  void draw_board();
  void draw_selection_board();
  void draw_pieces();

  // void display();

  void main_loop();



private:
  game my_game;  //This is where the game is

  GLuint vao;
  GLuint vbo;

  GLuint shader_program;

  int num_pts_board;
  //indexes for pieces?

  bool handle_input();


  std::vector<glm::vec3> points;    //add the 1.0 w value in the shader
  std::vector<glm::vec3> normals;   //represents surface orientation
  std::vector<glm::vec3> colors;   //represents surface color
  std::vector<glm::vec3> selection_colors; //represents surface for selection

  float rotation_of_board;
  float v_rotation_of_board;
  float rotation_of_light;

  // int white_space_start, white_space_num;
  // int black_space_start, black_space_num;

  int board_start, board_num;

  int pawn_start, pawn_num;
  int knight_start, knight_num;
  int bishop_start, bishop_num;
  int rook_start, rook_num;
  int queen_start, queen_num;
  int king_start, king_num;

  glm::vec3 white = glm::vec3(1,0.9,0.76);
  glm::vec3 black = glm::vec3(0.25,0.25,0);


  glm::vec3 offsets[8][8];  //used to place pieces more simply


  SDL_Window * window;
  SDL_GLContext context;


  bool selection_mode = false;

  static const int windowwidth = 1200;
  static const int windowheight = 700;


};



opengl_container::opengl_container()
{

  cout<<endl<<endl<<my_game.return_contents_at(1,1)<<endl;

  SDL_Init( SDL_INIT_VIDEO );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

  SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 8);

  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 5 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );



  window = SDL_CreateWindow( "OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowwidth, windowheight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
  context = SDL_GL_CreateContext( window );





  //DEBUG
  glEnable              ( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( MessageCallback, 0 );



  Shader s("resources/shaders/phong_vs.glsl", "resources/shaders/phong_fs.glsl");
  shader_program = s.Program;


  glUseProgram( shader_program );



  glEnable( GL_DEPTH_TEST );

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);



  glLineWidth(15.0);



  // glPointSize(16.0f);
  glPointSize(2.0f);

  glClearColor( 0.2, 0.1, 0.0, 0.0 );

  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );

  glGenBuffers( 1, &vbo );
  glBindBuffer( GL_ARRAY_BUFFER, vbo );



//clear out any data
  points.clear();
  normals.clear();
  colors.clear();
  selection_colors.clear();



//Then generate points/normals

  std::random_device rd;
  std::mt19937 mt(rd());

	std::uniform_real_distribution<float> dist1(-1.0f, 1.0f); //input values
	// std::uniform_int_distribution<int> dist2(0,nodes.size()-1);

  board_start = points.size();

  glm::vec3 up = glm::vec3(0,1,0);

  for(int i = 0; i < 64; i++)
  {
    int x = i % 8;
    int y = i / 8;

    if(y % 2 == 0)
    {
      if(x % 2 == 0)
      {
        for(int i = 0; i < 6; i++) colors.push_back(black);
      }
      else
      {
        for(int i = 0; i < 6; i++) colors.push_back(white);
      }
    }
    else
    {
      if(x % 2 == 0)
      {
        for(int i = 0; i < 6; i++) colors.push_back(white);
      }
      else
      {
        for(int i = 0; i < 6; i++) colors.push_back(black);
      }
    }

//normals + selection colors
    for(int i = 0; i < 6; i++)
    {
      normals.push_back(up);
      selection_colors.push_back(glm::vec3(0.125*x, 0.125*y, 0));
    }


    //  triangle 1 is ABC
    // A       B
    //  +-----+
    //  | 1 / |
    //  |  /  |
    //  | / 2 |
    //  +-----+
    // C       D
    //  triangle 2 is CBD


    //A : (n,n+1,-0.3);
    //B : (n+1,n+1,-0.3);
    //C : (n,n,-0.3);
    //D : (n+1,n,-0.3);

    glm::vec3 A = glm::vec3(-0.8+1.6*((float)(x)/8.0),-0.1,-0.8+1.6*((float)(y+1)/8.0));
    glm::vec3 B = glm::vec3(-0.8+1.6*((float)(x+1)/8.0),-0.1,-0.8+1.6*((float)(y+1)/8.0));
    glm::vec3 C = glm::vec3(-0.8+1.6*((float)(x)/8.0),-0.1,-0.8+1.6*((float)(y)/8.0));
    glm::vec3 D = glm::vec3(-0.8+1.6*((float)(x+1)/8.0),-0.1,-0.8+1.6*((float)(y)/8.0));

    offsets[x][y] = (A+B+C+D)/4.0f+glm::vec3(0,0.01,0);

    points.push_back(A);
    points.push_back(B);
    points.push_back(C);

    points.push_back(C);
    points.push_back(B);
    points.push_back(D);
  }






  //  triangle 1 is ABC
  // A       B
  //  +-----+
  //  | 1 / |
  //  |  /  |
  //  | / 2 |
  //  +-----+
  // C       D
  //  triangle 2 is CBD

  glm::vec3 A = glm::vec3(-0.8, -0.2, 0.8);
  glm::vec3 B = glm::vec3( 0.8, -0.2, 0.8);
  glm::vec3 C = glm::vec3(-0.9618, -0.1, 0.9618);
  glm::vec3 D = glm::vec3(0.9618, -0.1, 0.9618);

  points.push_back(A);
  points.push_back(C);
    points.push_back(B);

  points.push_back(C);
  points.push_back(D);
    points.push_back(B);



  // glm::vec3 gold = glm::vec3(0.8, 0.3, 0.05);
  glm::vec3 gold = glm::vec3(0.5,0.17,0.05);
  for(int i = 0; i < 6; i++) colors.push_back(gold);


  glm::vec3 norm = -glm::normalize(glm::cross(A-B, B-C));
  for(int i = 0; i < 6; i++) normals.push_back(-norm);


  glm::vec3 blue = glm::vec3(0.4, 0.5, 0.9);
  for(int i = 0; i < 6; i++) selection_colors.push_back(blue);









  A = glm::vec3(-0.8, -0.2, -0.8);
  B = glm::vec3(-0.8, -0.2, 0.8);
  C = glm::vec3(-0.9618, -0.1, -0.9618);
  D = glm::vec3(-0.9618, -0.1, 0.9618);

  points.push_back(A);
      points.push_back(C);
      points.push_back(B);

  points.push_back(C);
  points.push_back(D);
    points.push_back(B);


  for(int i = 0; i < 6; i++) colors.push_back(gold);

  norm = -glm::normalize(glm::cross(A-B, B-C));
  for(int i = 0; i < 6; i++) normals.push_back(-norm);

  for(int i = 0; i < 6; i++) selection_colors.push_back(blue);








  A = glm::vec3(0.8, -0.2, 0.8);
  B = glm::vec3(0.8, -0.2, -0.8);
  C = glm::vec3(0.9618, -0.1, 0.9618);
  D = glm::vec3(0.9618, -0.1, -0.9618);

  points.push_back(A);
      points.push_back(C);
      points.push_back(B);

  points.push_back(C);
      points.push_back(D);
      points.push_back(B);


  for(int i = 0; i < 6; i++) colors.push_back(gold);

  norm = -glm::normalize(glm::cross(A-B, B-C));
  for(int i = 0; i < 6; i++) normals.push_back(-norm);

  for(int i = 0; i < 6; i++) selection_colors.push_back(blue);












  A = glm::vec3(0.8, -0.2, -0.8);
  B = glm::vec3(-0.8, -0.2, -0.8);
  C = glm::vec3(0.9618, -0.1, -0.9618);
  D = glm::vec3(-0.9618, -0.1, -0.9618);

  points.push_back(A);
      points.push_back(C);
      points.push_back(B);

  points.push_back(C);
      points.push_back(D);
      points.push_back(B);


  for(int i = 0; i < 6; i++) colors.push_back(gold);

  norm = -glm::normalize(glm::cross(A-B, B-C));
  for(int i = 0; i < 6; i++) normals.push_back(-norm);

  for(int i = 0; i < 6; i++) selection_colors.push_back(blue);


  board_num = points.size() - board_start;




  std::vector<std::vector<std::vector< glm::vec3 >>> rotated_sections;
  rotated_sections.clear();
  rotated_sections.resize(6);   //6 pieces

  //indexing is done with three numbers - [piece][rotated slice][datapoints]

  //rotated_sections[0][0][...] is the original data for the pawn
  //rotated_sections[0][1][...] is the pawn data rotated by one increment
  //rotated_sections[0][2][...] is the pawn data rotated by two increments
  //...

  //so we want, say, 10 stops on the rotation
  int num_stops = 100;

  for(int i = 0; i < rotated_sections.size(); i++)
  {
    rotated_sections[i].resize(num_stops);   //each piece has num_stops slices
  }

  //order is:
    //  PAWN(0), KNIGHT(1), BISHOP(2), ROOK(3), QUEEN(4), KING(5)

  //push original information into
  // rotated_sections[piece][0][...] because 0 in the second index is the unrotated section


//pawn points
  rotated_sections[0][0].push_back(glm::vec3(0.0, 0.14, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.02, 0.135, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.03, 0.12, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.023, 0.10, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.018, 0.097, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.025, 0.095, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.03, 0.09, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.027, 0.085, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.017, 0.08, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.02, 0.05, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.05, 0.05, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.07, 0.02, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.062, 0.017, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.074, 0.015, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.071, -0.007, 0));
  rotated_sections[0][0].push_back(glm::vec3(0.064, -0.01, 0));


//knight points
  rotated_sections[1][0].push_back(glm::vec3(0.0, 0.18, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.037, 0.175, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.045, 0.17, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.038, 0.14, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.022, 0.135, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.017, 0.13, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.03, 0.11, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.032, 0.104, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.035, 0.10, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.04, 0.07, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.05, 0.05, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.07, 0.02, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.062, 0.017, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.074, 0.015, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.071, -0.007, 0));
  rotated_sections[1][0].push_back(glm::vec3(0.064, -0.01, 0));


//bishop points
  rotated_sections[2][0].push_back(glm::vec3(0.0, 0.21, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.01, 0.2, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.009, 0.194, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.02, 0.18, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.019, 0.178, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.024, 0.17, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.03, 0.15, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.03, 0.14, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.024, 0.13, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.026, 0.125, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.025, 0.122, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.0187, 0.12, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.027, 0.115, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.028, 0.113, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.04, 0.107, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.019 , 0.098, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.05, 0.05, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.07, 0.02, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.062, 0.017, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.074, 0.015, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.071, -0.007, 0));
  rotated_sections[2][0].push_back(glm::vec3(0.064, -0.01, 0));


//rook points
  rotated_sections[3][0].push_back(glm::vec3(0.0, 0.15, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.03, 0.15, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.03, 0.16, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.04, 0.16, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.04, 0.14, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.03, 0.135, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.024, 0.12, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.03, 0.06, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.05, 0.05, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.07, 0.02, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.062, 0.017, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.074, 0.015, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.071, -0.007, 0));
  rotated_sections[3][0].push_back(glm::vec3(0.064, -0.01, 0));


//queen points
  rotated_sections[4][0].push_back(glm::vec3(0.0, 0.24, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.015, 0.239, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.03, 0.24, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.035, 0.23, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.027, 0.22, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.025, 0.19, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.027, 0.187, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.025, 0.185, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.023, 0.17, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.032, 0.164, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.035, 0.16, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.042, 0.155, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.039, 0.148, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.017, 0.145, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.029, 0.065, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.034, 0.06, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.05, 0.05, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.07, 0.02, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.062, 0.017, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.074, 0.015, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.071, -0.007, 0));
  rotated_sections[4][0].push_back(glm::vec3(0.064, -0.01, 0));


//king points
  rotated_sections[5][0].push_back(glm::vec3(0.0, 0.31, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.01, 0.3, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.01, 0.29, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.02, 0.29, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.02, 0.27, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.015, 0.27, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.015, 0.257, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.017, 0.256, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.017, 0.254, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.019, 0.251, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.039, 0.25, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.041, 0.24, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.031, 0.2, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.034, 0.19, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.032, 0.185, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.04, 0.17, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.04, 0.165, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.054, 0.161, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.02, 0.15, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.035, 0.07, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.047, 0.065, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.049, 0.06, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.05, 0.05, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.07, 0.02, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.062, 0.017, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.074, 0.015, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.071, -0.007, 0));
  rotated_sections[5][0].push_back(glm::vec3(0.064, -0.01, 0));



  for(int i = 0; i < rotated_sections.size(); i++)  //iterating through pieces
  {
    for(int j = 1; j < num_stops; j++)            //iterating through slices
    {
      //this stop will have as many points in it as the first list of points
      rotated_sections[i][j].resize(rotated_sections[i][0].size());

      //read from the first slice, rotate based on 'j' increments, put back in index [i][j][...]
      for(int k = 0; k < rotated_sections[i][j].size(); k++)
      {
        // rotate the value from rotated_sections[i][0][k] by (360/num_stops)*k
        rotated_sections[i][j][k] = (glm::rotate(glm::radians(360.0f/((float)num_stops)*(float)j), glm::vec3(0,1,0)) * glm::vec4(rotated_sections[i][0][k], 1)).xyz();
      }
    }
  }







  pawn_start = points.size();


  //points and computed normals
  for(int i = 1; i < rotated_sections[0].size(); i++)         //iterate through slices
  {
    for(int j = 1; j < rotated_sections[0][i].size(); j++)    //iterate through points
    {
      glm::vec3 A, B, C, D, N;
      A = rotated_sections[0][i-1][j-1];
      B = rotated_sections[0][i][j-1];
      C = rotated_sections[0][i-1][j];
      D = rotated_sections[0][i][j];

      points.push_back(A);
      points.push_back(C);
      points.push_back(B);

      points.push_back(B);
      points.push_back(C);
      points.push_back(D);

      N = -glm::normalize(glm::cross(A-B, B-D));

      for(int k = 0; k < 6; k++)
      {
        normals.push_back(N);
      }
    }
  }

  //this is the last bit to stitch up the seam
  for(int j = 1; j < rotated_sections[0][rotated_sections[0].size()-1].size(); j++)
  {
    glm::vec3 A, B, C, D, N;
    A = rotated_sections[0][rotated_sections[0].size()-1][j-1];
    B = rotated_sections[0][0][j-1];
    C = rotated_sections[0][rotated_sections[0].size()-1][j];
    D = rotated_sections[0][0][j];

    points.push_back(A);
    points.push_back(C);
    points.push_back(B);

    points.push_back(B);
    points.push_back(C);
    points.push_back(D);

    N = -glm::normalize(glm::cross(A-B, B-D));

    for(int k = 0; k < 6; k++)
    {
      normals.push_back(N);
    }
  }

  pawn_num = points.size() - pawn_start;
  cout << endl << "pawn consists of " << pawn_num << " points" << endl << endl;







  knight_start = points.size();
  //knight points

  //points and computed normals
  for(int i = 1; i < rotated_sections[1].size(); i++)         //iterate through slices
  {
    for(int j = 1; j < rotated_sections[1][i].size(); j++)    //iterate through points
    {
      glm::vec3 A, B, C, D, N;
      A = rotated_sections[1][i-1][j-1];
      B = rotated_sections[1][i][j-1];
      C = rotated_sections[1][i-1][j];
      D = rotated_sections[1][i][j];

      points.push_back(A);
      points.push_back(C);
      points.push_back(B);

      points.push_back(B);
      points.push_back(C);
      points.push_back(D);

      N = -glm::normalize(glm::cross(A-B, B-D));

      for(int k = 0; k < 6; k++)
      {
        normals.push_back(N);
      }
    }
  }

  //this is the last bit to stitch up the seam
  for(int j = 1; j < rotated_sections[1][rotated_sections[1].size()-1].size(); j++)
  {
    glm::vec3 A, B, C, D, N;
    A = rotated_sections[1][rotated_sections[1].size()-1][j-1];
    B = rotated_sections[1][0][j-1];
    C = rotated_sections[1][rotated_sections[1].size()-1][j];
    D = rotated_sections[1][0][j];

    points.push_back(A);
    points.push_back(C);
    points.push_back(B);

    points.push_back(B);
    points.push_back(C);
    points.push_back(D);

    N = -glm::normalize(glm::cross(A-B, B-D));
    cout << N[0] << " " << N[1] << " " << N[2] << endl;

    for(int k = 0; k < 6; k++)
    {
      normals.push_back(N);
    }
  }

  knight_num = points.size() - knight_start;
  cout << endl << "knight consists of " << knight_num << " points" << endl << endl;




  bishop_start = points.size();
  //bishop points

  //points and computed normals
  for(int i = 1; i < rotated_sections[2].size(); i++)         //iterate through slices
  {
    for(int j = 1; j < rotated_sections[2][i].size(); j++)    //iterate through points
    {
      glm::vec3 A, B, C, D, N;
      A = rotated_sections[2][i-1][j-1];
      B = rotated_sections[2][i][j-1];
      C = rotated_sections[2][i-1][j];
      D = rotated_sections[2][i][j];

      points.push_back(A);
      points.push_back(C);
      points.push_back(B);

      points.push_back(B);
      points.push_back(C);
      points.push_back(D);

      N = -glm::normalize(glm::cross(A-B, B-D));

      for(int k = 0; k < 6; k++)
      {
        normals.push_back(N);
      }
    }
  }

  //this is the last bit to stitch up the seam
  for(int j = 1; j < rotated_sections[2][rotated_sections[2].size()-1].size(); j++)
  {
    glm::vec3 A, B, C, D, N;
    A = rotated_sections[2][rotated_sections[2].size()-1][j-1];
    B = rotated_sections[2][0][j-1];
    C = rotated_sections[2][rotated_sections[2].size()-1][j];
    D = rotated_sections[2][0][j];

    points.push_back(A);
    points.push_back(C);
    points.push_back(B);

    points.push_back(B);
    points.push_back(C);
    points.push_back(D);

    N = -glm::normalize(glm::cross(A-B, B-D));

    for(int k = 0; k < 6; k++)
    {
      normals.push_back(N);
    }
  }

  bishop_num = points.size() - bishop_start;
  cout << endl << "bishop consists of " << bishop_num << " points" << endl << endl;



  rook_start = points.size();
  //rook points

  //points and computed normals
  for(int i = 1; i < rotated_sections[3].size(); i++)         //iterate through slices
  {
    for(int j = 1; j < rotated_sections[3][i].size(); j++)    //iterate through points
    {
      glm::vec3 A, B, C, D, N;
      A = rotated_sections[3][i-1][j-1];
      B = rotated_sections[3][i][j-1];
      C = rotated_sections[3][i-1][j];
      D = rotated_sections[3][i][j];

      points.push_back(A);
      points.push_back(C);
      points.push_back(B);

      points.push_back(B);
      points.push_back(C);
      points.push_back(D);

      N = -glm::normalize(glm::cross(A-B, B-D));


      for(int k = 0; k < 6; k++)
      {
        normals.push_back(N);
      }
    }
  }

  //this is the last bit to stitch up the seam
  for(int j = 1; j < rotated_sections[3][rotated_sections[3].size()-1].size(); j++)
  {
    glm::vec3 A, B, C, D, N;
    A = rotated_sections[3][rotated_sections[3].size()-1][j-1];
    B = rotated_sections[3][0][j-1];
    C = rotated_sections[3][rotated_sections[3].size()-1][j];
    D = rotated_sections[3][0][j];

    points.push_back(A);
    points.push_back(C);
    points.push_back(B);

    points.push_back(B);
    points.push_back(C);
    points.push_back(D);

    N = -glm::normalize(glm::cross(A-B, B-D));

    for(int k = 0; k < 6; k++)
    {
      normals.push_back(N);
    }
  }
  rook_num = points.size() - rook_start;
  cout << endl << "rook consists of " << rook_num << " points" << endl << endl;



  queen_start = points.size();
  //queen points

  //points and computed normals
  for(int i = 1; i < rotated_sections[4].size(); i++)         //iterate through slices
  {
    for(int j = 1; j < rotated_sections[4][i].size(); j++)    //iterate through points
    {
      glm::vec3 A, B, C, D, N;
      A = rotated_sections[4][i-1][j-1];
      B = rotated_sections[4][i][j-1];
      C = rotated_sections[4][i-1][j];
      D = rotated_sections[4][i][j];

      points.push_back(A);
      points.push_back(C);
      points.push_back(B);

      points.push_back(B);
      points.push_back(C);
      points.push_back(D);

      N = -glm::normalize(glm::cross(A-B, B-D));

      for(int k = 0; k < 6; k++)
      {
        normals.push_back(N);
      }
    }
  }

  //this is the last bit to stitch up the seam
  for(int j = 1; j < rotated_sections[4][rotated_sections[4].size()-1].size(); j++)
  {
    glm::vec3 A, B, C, D, N;
    A = rotated_sections[4][rotated_sections[4].size()-1][j-1];
    B = rotated_sections[4][0][j-1];
    C = rotated_sections[4][rotated_sections[4].size()-1][j];
    D = rotated_sections[4][0][j];

    points.push_back(A);
    points.push_back(C);
    points.push_back(B);

    points.push_back(B);
    points.push_back(C);
    points.push_back(D);

    N = -glm::normalize(glm::cross(A-B, B-D));

    for(int k = 0; k < 6; k++)
    {
      normals.push_back(N);
    }
  }

  queen_num = points.size() - queen_start;
  cout << endl << "queen consists of " << queen_num << " points" << endl << endl;



  king_start = points.size();
  //king points

  //points and computed normals
  for(int i = 1; i < rotated_sections[5].size(); i++)         //iterate through slices
  {
    for(int j = 1; j < rotated_sections[5][i].size(); j++)    //iterate through points
    {
      glm::vec3 A, B, C, D, N;
      A = rotated_sections[5][i-1][j-1];
      B = rotated_sections[5][i][j-1];
      C = rotated_sections[5][i-1][j];
      D = rotated_sections[5][i][j];

      points.push_back(A);
      points.push_back(C);
      points.push_back(B);

      points.push_back(B);
      points.push_back(C);
      points.push_back(D);

      N = -glm::normalize(glm::cross(A-B, B-D));

      for(int k = 0; k < 6; k++)
      {
        normals.push_back(N);
      }
    }
  }

  //this is the last bit to stitch up the seam
  for(int j = 1; j < rotated_sections[5][rotated_sections[5].size()-1].size(); j++)
  {
    glm::vec3 A, B, C, D, N;
    A = rotated_sections[5][rotated_sections[5].size()-1][j-1];
    B = rotated_sections[5][0][j-1];
    C = rotated_sections[5][rotated_sections[5].size()-1][j];
    D = rotated_sections[5][0][j];

    points.push_back(A);
    points.push_back(C);
    points.push_back(B);

    points.push_back(B);
    points.push_back(C);
    points.push_back(D);

    N = -glm::normalize(glm::cross(A-B, B-D));

    for(int k = 0; k < 6; k++)
    {
      normals.push_back(N);
    }
  }
  king_num = points.size() - king_start;
  cout << endl << "king consists of " << king_num << " points" << endl << endl;


  for(int i = 0; i < normals.size(); i++)
  {
    if(normals[i] == glm::vec3(0,0,0) || glm::all(glm::isnan(normals[i])))
      normals[i] = glm::vec3(0,1,0);

    cout << normals[i][0]  << " " << normals[i][1] << " " << normals[i][2] << endl;
  }











  const GLuint num_bytes_points           = sizeof(glm::vec3) * points.size();
  // cout << endl << num_bytes_points << endl;

  const GLuint num_bytes_normals          = sizeof(glm::vec3) * normals.size();
  // cout << num_bytes_normals << endl;

  const GLuint num_bytes_colors           = sizeof(glm::vec3) * colors.size();
  // cout << num_bytes_colors << endl;

  const GLuint num_bytes_selection_colors = sizeof(glm::vec3) * selection_colors.size();
  // cout << num_bytes_selection_colors << endl;



  GLint num_bytes = num_bytes_points + num_bytes_normals + num_bytes_colors + num_bytes_selection_colors;
  // cout << endl << " " << num_bytes << endl << endl;

  glBufferData(GL_ARRAY_BUFFER, num_bytes, NULL, GL_STATIC_DRAW);




  cout << "setting up points attrib" << endl << std::flush;
  glBufferSubData(GL_ARRAY_BUFFER, 0, num_bytes_points, &points[0]);
  glEnableVertexAttribArray(glGetAttribLocation(shader_program, "i_position"));
  glVertexAttribPointer(glGetAttribLocation(shader_program, "i_position"), 3, GL_FLOAT, false, 0, (static_cast<const char*>(0)));

  cout << "setting up normals attrib" << endl << std::flush;
  glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points, num_bytes_normals, &normals[0]);
  glEnableVertexAttribArray(glGetAttribLocation(shader_program, "i_normal"));
  glVertexAttribPointer(glGetAttribLocation(shader_program, "i_normal"), 3, GL_FLOAT, false, 0, (static_cast<const char*>(0) + (num_bytes_points)));

  cout << "setting up colors attrib" << endl << std::flush;
  glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points+num_bytes_normals, num_bytes_colors, &colors[0]);
  glEnableVertexAttribArray(glGetAttribLocation(shader_program, "i_color"));
  glVertexAttribPointer(glGetAttribLocation(shader_program, "i_color"), 3, GL_FLOAT, false, 0, (static_cast<const char*>(0) + (num_bytes_points+num_bytes_normals)));

  cout << "setting up selection colors attrib" << endl << std::flush;
  glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points+num_bytes_normals+num_bytes_colors, num_bytes_selection_colors, &selection_colors[0]);
  glEnableVertexAttribArray(glGetAttribLocation(shader_program, "i_sel_color"));
  glVertexAttribPointer(glGetAttribLocation(shader_program, "i_sel_color"), 3, GL_FLOAT, false, 0, (static_cast<const char*>(0) + (num_bytes_points+num_bytes_normals+num_bytes_colors)));



  // GLfloat left = -1.366f;
  // GLfloat right = 1.366f;
  // GLfloat top = -0.768f;
  // GLfloat bottom = 0.768f;
  // GLfloat zNear = 1.2f;
  // GLfloat zFar = -1.0f;
  // glm::mat4 proj = glm::ortho(left, right, top, bottom, zNear, zFar);


  //convert to perspective
  glm::mat4 proj = glm::perspective(glm::radians(50.0f), 1366.0f / 768.0f, 0.25f, 6.0f);
  glUniformMatrix4fv( glGetUniformLocation( shader_program, "u_projection_matrix" ), 1, GL_FALSE, glm::value_ptr(proj) );


  glm::mat4 view = glm::lookAt(
      glm::vec3(0.0f, 1.2f, -1.9f),
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f)
  );
  glUniformMatrix4fv( glGetUniformLocation( shader_program, "u_view_matrix" ), 1, GL_FALSE, glm::value_ptr(view) );




  glUniform1i(glGetUniformLocation( shader_program, "mode" ), 0);




  rotation_of_board = 0.1*SDL_GetTicks();
  glUniform1fv(glGetUniformLocation(shader_program, "rot"), 1, &rotation_of_board);
  v_rotation_of_board = 0.1*SDL_GetTicks();
  glUniform1fv(glGetUniformLocation(shader_program, "vrot"), 1, &v_rotation_of_board);
  rotation_of_light = 0.1*SDL_GetTicks();
  glUniform1fv(glGetUniformLocation(shader_program, "light_rotation"), 1, &rotation_of_light);



  glm::vec4 color = glm::vec4(0,0,0,1);
  glUniform4fv(glGetUniformLocation(shader_program, "u_color"), 1, glm::value_ptr(color));

}

opengl_container::~opengl_container()
{
  SDL_GL_DeleteContext( context );
  SDL_DestroyWindow( window );
  SDL_Quit();
}


// void opengl_container::update_rotation()
// {
//   rotation_of_board = 0.1*SDL_GetTicks();
//   glUniform1fv(glGetUniformLocation(shader_program, "rot"), 1, &rotation_of_board);
//   glUniform1fv(glGetUniformLocation(shader_program, "light_rotation"), 1, &rotation_of_board);
//
// }



bool opengl_container::handle_input()
{
  SDL_Event e;
  bool quit = false;

  while( SDL_PollEvent( &e ) )
  {
    //User requests quit - this is through the x or keyboard shortcut
    if( e.type == SDL_QUIT )
    {//this type means you got a message to quit
      // ┌─┐┬ ┬┌─┐┌┬┐┌─┐┌┬┐  ┌─┐─┐ ┬┬┌┬┐
      // └─┐└┬┘└─┐ │ ├┤ │││  ├┤ ┌┴┬┘│ │
      // └─┘ ┴ └─┘ ┴ └─┘┴ ┴  └─┘┴ └─┴ ┴

      cout << "quitting via kill";  //this is called when exiting via key combo/window x button
      quit = true;  //this will be seen from main
    }
    else if( e.type == SDL_KEYDOWN )
    {// this type means you pressed a key
      //  ┬┌─┌─┐┬ ┬  ┌─┐┬─┐┌─┐┌─┐┌─┐
      //  ├┴┐├┤ └┬┘  ├─┘├┬┘├┤ └─┐└─┐
      //  ┴ ┴└─┘ ┴   ┴  ┴└─└─┘└─┘└─┘

      //INFO:
      // https://wiki.libsdl.org/SDL_KeyboardEvent

      //switch based on key press
      switch( e.key.keysym.sym )
      {
        case SDLK_d:
          cout << "  d  key pressed";
          selection_mode = true;
          break;

        case SDLK_UP:
          cout << "  up  key pressed" << v_rotation_of_board;
          v_rotation_of_board+=30;
          glUniform1fv(glGetUniformLocation(shader_program, "vrot"), 1, &v_rotation_of_board);

          break;

        case SDLK_DOWN:
          cout << "  down  key pressed";
          v_rotation_of_board-=30;
          glUniform1fv(glGetUniformLocation(shader_program, "vrot"), 1, &v_rotation_of_board);

          break;

        case SDLK_LEFT:
          cout << "  left  key pressed";
          rotation_of_board+=30;
          glUniform1fv(glGetUniformLocation(shader_program, "rot"), 1, &rotation_of_board);
          break;

        case SDLK_RIGHT:
          cout << "  right  key pressed";
          rotation_of_board-=30;
          glUniform1fv(glGetUniformLocation(shader_program, "rot"), 1, &rotation_of_board);
          break;





        case SDLK_a:
          rotation_of_light+=30;
          glUniform1fv(glGetUniformLocation(shader_program, "light_rotation"), 1, &rotation_of_light);

          break;




        case SDLK_s:
          rotation_of_light-=30;
          glUniform1fv(glGetUniformLocation(shader_program, "light_rotation"), 1, &rotation_of_light);

          break;






        case SDLK_ESCAPE:
          cout << " exiting (quitting via escape)";   // this is called when the escape button is hit
          quit = true;  //this will be seen in main
          break;

        case SDLK_f:
          cout << " going fullscreen";
          SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
          break;

        default:
          // cout << " some other key pressed - ";
          // cout << "scancode is: " << SDL_GetScancodeName(e.key.keysym.scancode);
          // cout <<" with name: " <<  SDL_GetKeyName(e.key.keysym.sym);
          cout << SDL_GetKeyName(e.key.keysym.sym) << " down at ";
          break;
      }
      if(e.key.repeat)
        cout << " is being held ";
    }
    else if(e.type == SDL_KEYUP)
    { //this type means that you released a key
      //  ┬┌─┌─┐┬ ┬  ┬─┐┌─┐┬  ┌─┐┌─┐┌─┐┌─┐
      //  ├┴┐├┤ └┬┘  ├┬┘├┤ │  ├┤ ├─┤└─┐├┤
      //  ┴ ┴└─┘ ┴   ┴└─└─┘┴─┘└─┘┴ ┴└─┘└─┘

      switch( e.key.keysym.sym )
      {
        case SDLK_d:
          cout << "  d  key let go";
          selection_mode = false;
          break;
        default:
          cout << SDL_GetKeyName(e.key.keysym.sym) << " up at ";
          break;
      }
    }
    else if(e.type == SDL_MOUSEBUTTONDOWN)
    {//you clicked the mouse button down, at some xy - note that y goes from 0 at the top to whatever value at the bottom
      //  ┌┬┐┌─┐┬ ┬┌─┐┌─┐  ┌─┐┬─┐┌─┐┌─┐┌─┐
      //  ││││ ││ │└─┐├┤   ├─┘├┬┘├┤ └─┐└─┐
      //  ┴ ┴└─┘└─┘└─┘└─┘  ┴  ┴└─└─┘└─┘└─┘
      switch(e.button.button) //this is stupid notation
      {
        case SDL_BUTTON_LEFT:
          cout << "left click at x:" << e.button.x << " y:" << e.button.y;

          unsigned char pixel[4];
          glReadPixels(e.button.x, windowheight - e.button.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
          // int x = e.button.x;  //crosses initialization
          //my_game.handle_click();

          cout << endl << endl << " clicked on:  r:" << (int)pixel[0] << " g:" << (int)pixel[1] << " b:" << (int)pixel[2] << " a:" << (int)pixel[3] << endl << endl;

          break;

        case SDL_BUTTON_MIDDLE:
          cout << "middle click at x:" << e.button.x << " y:" << e.button.y;
          break;

        case SDL_BUTTON_RIGHT:
          cout << "right click at x:" << e.button.x << " y:" << e.button.y;
          break;
      }
    }
    else if(e.type == SDL_MOUSEBUTTONUP)
    {//you let go of a mouse button, at some xy - again, same deal with y being inverted
      //  ┌┬┐┌─┐┬ ┬┌─┐┌─┐  ┬─┐┌─┐┬  ┌─┐┌─┐┌─┐┌─┐
      //  ││││ ││ │└─┐├┤   ├┬┘├┤ │  ├┤ ├─┤└─┐├┤
      //  ┴ ┴└─┘└─┘└─┘└─┘  ┴└─└─┘┴─┘└─┘┴ ┴└─┘└─┘
      switch(e.button.button)
      {
        case SDL_BUTTON_LEFT:
          cout << "left click released at x:" << e.button.x << " y:" << e.button.y;

          unsigned char pixel[4];
          glReadPixels(e.button.x, windowheight - e.button.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

          cout << endl << endl << " released on:  r:" << (int)pixel[0] << " g:" << (int)pixel[1] << " b:" << (int)pixel[2] << " a:" << (int)pixel[3] << endl << endl;

          break;

        case SDL_BUTTON_MIDDLE:
          cout << "middle click released at x:" << e.button.x << " y:" << e.button.y;
          break;

        case SDL_BUTTON_RIGHT:
          cout << "right click released at x:" << e.button.x << " y:" << e.button.y;
          break;
      }
    }
    cout << " at time " << e.key.timestamp << endl;
  }
  return quit;
}


void opengl_container::draw_board()
{
  glUniform1i(glGetUniformLocation( shader_program, "mode" ), 0);
  glDrawArrays(GL_TRIANGLES, board_start, board_num);
}

void opengl_container::draw_selection_board()
{
  glUniform1i(glGetUniformLocation( shader_program, "mode" ), 4);
  glDrawArrays(GL_TRIANGLES, board_start, board_num);
}

void opengl_container::draw_pieces()
{

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(0,14);


  glm::vec3 offset;
  glm::vec4 selection_color;


  for(int x = 0; x < 8; x++)
  {
    for(int y = 0; y < 8; y++)
    {

      switch(my_game.return_contents_at(x, y))
      {
          case blackpawn:
          case blackknight:
          case blackbishop:
          case blackrook:
          case blackqueen:
          case blackking:
              offset = offsets[x][y];
              glUniform3fv(glGetUniformLocation( shader_program, "u_offset" ),1, glm::value_ptr(offset) );

              selection_color = glm::vec4(0.125*x, 0.125*y, 0, 1);
              glUniform4fv(glGetUniformLocation(shader_program, "u_color"), 1, glm::value_ptr(selection_color));

              if(selection_mode)
                glUniform1i(glGetUniformLocation( shader_program, "mode" ), 5);
              else
                glUniform1i(glGetUniformLocation( shader_program, "mode" ), 1);
            break;


          case whitepawn:
          case whiteknight:
          case whitebishop:
          case whiterook:
          case whitequeen:
          case whiteking:
              offset = offsets[x][y];
              glUniform3fv(glGetUniformLocation( shader_program, "u_offset" ),1, glm::value_ptr(offset) );

              selection_color = glm::vec4(0.125*x, 0.125*y, 0, 1);
              glUniform4fv(glGetUniformLocation(shader_program, "u_color"), 1, glm::value_ptr(selection_color));

              if(selection_mode)
                glUniform1i(glGetUniformLocation( shader_program, "mode" ), 5);
              else
                glUniform1i(glGetUniformLocation( shader_program, "mode" ), 2);
            break;
      }






      switch(my_game.return_contents_at(x, y))
      {
        case whitepawn:
        case blackpawn:
          glDrawArrays(GL_TRIANGLES, pawn_start, pawn_num);
          break;

        case whiteknight:
        case blackknight:
          glDrawArrays(GL_TRIANGLES, knight_start, knight_num);
          break;

        case whitebishop:
        case blackbishop:
          glDrawArrays(GL_TRIANGLES, bishop_start, bishop_num);
          break;

        case whiterook:
        case blackrook:
          glDrawArrays(GL_TRIANGLES, rook_start, rook_num);
          break;

        case whitequeen:
        case blackqueen:
          glDrawArrays(GL_TRIANGLES, queen_start, queen_num);
          break;

        case whiteking:
        case blackking:
          glDrawArrays(GL_TRIANGLES, king_start, king_num);
          break;
      }














































      // switch(dist(mt))
      // {
      //   case blackpawn:
      //   case blackknight:
      //   case blackbishop:
      //   case blackrook:
      //   case blackqueen:
      //   case blackking:
      //     offset = offsets[x][y];
      //     glUniform3fv(glGetUniformLocation( shader_program, "u_offset" ),1, glm::value_ptr(offset) );
      //
      //     selection_color = glm::vec4(0.125*x, 0.125*y, 0, 1);
      //     glUniform4fv(glGetUniformLocation(shader_program, "u_color"), 1, glm::value_ptr(selection_color));
      //
      //     if(selection_mode)
      //       glUniform1i(glGetUniformLocation( shader_program, "mode" ), 5);
      //     else
      //       glUniform1i(glGetUniformLocation( shader_program, "mode" ), 1);
      //
      //     glDrawArrays(GL_LINES, pawn_start, pawn_num);
      //     break;
      //
      //   case whitepawn:
      //   case whiteknight:
      //   case whitebishop:
      //   case whiterook:
      //   case whitequeen:
      //   case whiteking:
      //     offset = offsets[x][y];
      //     glUniform3fv(glGetUniformLocation( shader_program, "u_offset" ),1, glm::value_ptr(offset) );
      //
      //     selection_color = glm::vec4(0.125*x, 0.125*y, 0, 1);
      //     glUniform4fv(glGetUniformLocation(shader_program, "u_color"), 1, glm::value_ptr(selection_color));
      //
      //     if(selection_mode)
      //       glUniform1i(glGetUniformLocation( shader_program, "mode" ), 5);
      //     else
      //       glUniform1i(glGetUniformLocation( shader_program, "mode" ), 2);
      //
      //     glDrawArrays(GL_LINES, pawn_start, pawn_num);
      //     break;
      // }




















      // if(y<2)
      // {
      //   offset = offsets[x][y];
      //   glUniform3fv(glGetUniformLocation( shader_program, "u_offset" ),1, glm::value_ptr(offset) );
      //
      //   selection_color = glm::vec4(0.125*x, 0.125*y, 0, 1);
      //   glUniform4fv(glGetUniformLocation(shader_program, "u_color"), 1, glm::value_ptr(selection_color));
      //
      //   if(selection_mode)
      //   {
      //     glUniform1i(glGetUniformLocation( shader_program, "mode" ), 5);
      //   }
      //   else
      //   {
      //     glUniform1i(glGetUniformLocation( shader_program, "mode" ), 1);
      //   }
      //
      //   // glDrawArrays(GL_TRIANGLES, pawn_start, pawn_num);
      //   glDrawArrays(GL_TRIANGLES, bishop_start, bishop_num);
      // }
      //
      //
      //
      //
      // if(y>5)
      // {
      //   offset = offsets[x][y];
      //   glUniform3fv(glGetUniformLocation( shader_program, "u_offset" ),1, glm::value_ptr(offset) );
      //
      //   selection_color = glm::vec4(0.125*x, 0.125*y, 0, 1);
      //   glUniform4fv(glGetUniformLocation(shader_program, "u_color"), 1, glm::value_ptr(selection_color));
      //
      //   if(selection_mode)
      //   {
      //     glUniform1i(glGetUniformLocation( shader_program, "mode" ), 5);
      //   }
      //   else
      //   {
      //     glUniform1i(glGetUniformLocation( shader_program, "mode" ), 2);
      //   }
      //
      //   glDrawArrays(GL_TRIANGLES, pawn_start, pawn_num);
      //
      // }
    }
  }

  glUniform1i(glGetUniformLocation( shader_program, "mode" ), 0);

  glm::vec3 default_offset = glm::vec3(0,0,0);
  glUniform3fv(glGetUniformLocation( shader_program, "u_offset" ),1, glm::value_ptr(default_offset) );

}

void opengl_container::main_loop()
{

  while(!handle_input())
  {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // update_rotation();

    if(selection_mode)
      draw_selection_board();
    else
      draw_board();

    draw_pieces();

    SDL_GL_SwapWindow( window );
    SDL_Delay(20);
  }
}


#endif
