#ifndef VORALDO
#define VORALDO

#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <vector>
#include <stdio.h>
#include <numeric>
#include <string>
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

#include "lodepng.h"
// Good, simple png library


#include <SDL.h>
#include <SDL_opengl.h>     //allows you to run OpenGL inside of SDL2
#include <SDL2/SDL_ttf.h>   //allows rendering of text using .ttf font files (SDL 2d renderer only)



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

//  ╔═╗┌─┐┬─┐┬  ┬┌┐┌
//  ╠═╝├┤ ├┬┘│  ││││
//  ╩  └─┘┴└─┴─┘┴┘└┘

class PerlinNoise
{
	// The permutation vector
	std::vector<int> p;
public:
	// Initialize with the reference values for the permutation vector
	PerlinNoise( );
	// Generate a new permutation vector based on the value of seed
	PerlinNoise( unsigned int seed );
	// Get a noise value, for 2D images z can have any value
	double noise( double x, double y, double z );
private:
	double fade( double t );
	double lerp( double t, double a, double b );
	double grad( int hash, double x, double y, double z );
};

//  ╦  ╦┌─┐┬─┐┌─┐┬  ┌┬┐┌─┐
//  ╚╗╔╝│ │├┬┘├─┤│   │││ │
//   ╚╝ └─┘┴└─┴ ┴┴─┘─┴┘└─┘

class voraldo
{
public:
  voraldo();
  ~voraldo();

private:

  void startup_info_dump();

  void take_input();
  void draw_menu();

//  ╔╦╗┌─┐┌┐┌┬ ┬  ╦ ╦┌─┐┌┐┌┌┬┐┬  ┬┌┐┌┌─┐
//  ║║║├┤ ││││ │  ╠═╣├─┤│││ │││  │││││ ┬
//  ╩ ╩└─┘┘└┘└─┘  ╩ ╩┴ ┴┘└┘─┴┘┴─┘┴┘└┘└─┘

  typedef enum menu_state_t
  {
    MAIN_MENU,    //top level menu - go to this from the splash

    DRAW_MENU,    //first level submenus
    MASK_MENU,
    UTIL_MENU,

    SPHERE_CONFIG

    //primitive config submenus

    //...
  } menu_state;

  menu_state current_menu_state;  //so we know where we are when handling new input
  bool quit;

//  ╔═╗┌─┐┌┬┐┌┬┐┌─┐┌┐┌  ╔═╗╔╦╗╦    ┌─┐┌┬┐┬ ┬┌─┐┌─┐
//  ║  │ ││││││││ ││││  ╚═╗ ║║║    └─┐ │ │ │├┤ ├┤
//  ╚═╝└─┘┴ ┴┴ ┴└─┘┘└┘  ╚═╝═╩╝╩═╝  └─┘ ┴ └─┘└  └

  int total_screen_width, total_screen_height;

  SDL_Rect SrcRect  = {0,0,720,405};   //where are we taking pixels from when we load the splash screen
  SDL_Rect DestRect = {0,0,720,405};  //where are we putting the pixels we took from SrcRect?

  void sdl_ttf_init();              //set up the font and everything
  TTF_Font *font;                  //the loaded font


  void font_test();
  void ttf_string(std::string s, int basex, int basey, unsigned char r, unsigned char g, unsigned char b);


  SDL_Renderer* SDL_2D_renderer;

  std::string splash_path = std::string("resources/splash.bmp");
  SDL_Texture* splash;           //this is the start splash screen
  SDL_Surface* splashBMP;


  std::string exit_splash_path = std::string("resources/exit_splash.bmp");
  SDL_Texture* exit_splash;   //this is the ending splash screen (could just do it with SDL_ttf)
  SDL_Surface* exit_splashBMP;





  //  ╔═╗┌─┐┌─┐┌┐┌╔═╗╦    ╦ ╦┬┌┐┌┌┬┐┌─┐┬ ┬
  //  ║ ║├─┘├┤ │││║ ╦║    ║║║││││ │││ ││││
  //  ╚═╝┴  └─┘┘└┘╚═╝╩═╝  ╚╩╝┴┘└┘─┴┘└─┘└┴┘

  //this window plays host to all the hardware accelerated 3d graphics, and the GL context
  SDL_Window * OpenGL_window;
  SDL_GLContext GLcontext;

  //I want to split the screen into three sections so I need to read out screen resolution
  int windowwidth;
  int windowheight;

  void create_gl_window();


  //  ╦┌┐┌┌─┐┌─┐  ╦ ╦┬┌┐┌┌┬┐┌─┐┬ ┬
  //  ║│││├┤ │ │  ║║║││││ │││ ││││
  //  ╩┘└┘└  └─┘  ╚╩╝┴┘└┘─┴┘└─┘└┴┘

  //this window will display images using the SDL 2d renderer, as well as buttons
  SDL_Window * Informational_window;

  int Infowindowwidth;
  int Infowindowheight;

  void create_info_window();

};


#endif
