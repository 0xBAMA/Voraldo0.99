/*
    Minimal SDL2 + OpenGL3 example.
    Author: https://github.com/koute
    This file is in the public domain; you can do whatever you want with it.
    In case the concept of public domain doesn't exist in your jurisdiction
    you can also use this code under the terms of Creative Commons CC0 license,
    either version 1.0 or (at your option) any later version; for details see:
        http://creativecommons.org/publicdomain/zero/1.0/
    This software is distributed without any warranty whatsoever.
    Compile and run with: gcc opengl3_hello.c `sdl2-config --libs --cflags` -lGL -Wall && ./a.out
*/

#define GL_GLEXT_PROTOTYPES

#include <SDL.h>
#include <SDL_opengl.h>

#include <stdio.h>
#include <iostream>
#include <random>


using std::cout;
using std::cerr;
using std::endl;

typedef float t_mat4x4[16];

static inline void mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar )
{
    #define T(a, b) (a * 4 + b)

    out[T(0,0)] = 2.0f / (right - left);
    out[T(0,1)] = 0.0f;
    out[T(0,2)] = 0.0f;
    out[T(0,3)] = 0.0f;

    out[T(1,1)] = 2.0f / (top - bottom);
    out[T(1,0)] = 0.0f;
    out[T(1,2)] = 0.0f;
    out[T(1,3)] = 0.0f;

    out[T(2,2)] = -2.0f / (zfar - znear);
    out[T(2,0)] = 0.0f;
    out[T(2,1)] = 0.0f;
    out[T(2,3)] = 0.0f;

    out[T(3,0)] = -(right + left) / (right - left);
    out[T(3,1)] = -(top + bottom) / (top - bottom);
    out[T(3,2)] = -(zfar + znear) / (zfar - znear);
    out[T(3,3)] = 1.0f;

    #undef T
}

static const char * vertex_shader =
    "#version 450\n"
    "in vec2 i_position;\n"
    "in vec4 i_color;\n"
    "out vec4 v_color;\n"
    "uniform float offset;\n"
    "uniform mat4 u_projection_matrix;\n"
    "void main() {\n"
    "    v_color = i_color;\n"
    "    gl_Position = u_projection_matrix * vec4( i_position, 0.0, 1.0 );\n"
    "}\n";

static const char * fragment_shader =
    "#version 450\n"
    "in vec4 v_color;\n"
    "out vec4 o_color;\n"
    "void main() {\n"
    "    o_color = v_color;\n"
    "}\n";

typedef enum t_attrib_id
{
    attrib_position,
    attrib_color
} t_attrib_id;

int main( int argc, char * argv[] )
{
  bool aligned = false;



  SDL_Window* win;
  SDL_Renderer* ren;
  SDL_Texture* tex;
  SDL_Texture* splash;




  if (SDL_Init(SDL_INIT_EVERYTHING) != 0){ cerr << "SDL_Init Error: " << SDL_GetError() << endl; return EXIT_FAILURE; }

  win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 405, SDL_WINDOW_OPENGL);
  if (win == NULL){ cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl; return EXIT_FAILURE; }

  ren  = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (ren == NULL){ cerr << "SDL_CreateRenderer Error" << SDL_GetError() << endl; return EXIT_FAILURE; }


  SDL_Surface* bmp1 = NULL;
  SDL_Surface* bmp2 = NULL;






  bmp1 = SDL_LoadBMP("grumpy-cat.bmp");
  if (bmp1 == NULL){ cerr << "SDL_LoadBMP Error: " << SDL_GetError() << endl; return EXIT_FAILURE; }

  tex = SDL_CreateTextureFromSurface(ren, bmp1);
  SDL_FreeSurface(bmp1);
  if (tex == NULL){ cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << endl; return EXIT_FAILURE; }


  bmp2 = SDL_LoadBMP("splash.bmp");
  if (bmp2 == NULL){ cerr << "SDL_LoadBMP Error: " << SDL_GetError() << endl; return EXIT_FAILURE; }

  splash = SDL_CreateTextureFromSurface(ren, bmp2);
  SDL_FreeSurface(bmp2);
  if (splash == NULL){ cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << endl; return EXIT_FAILURE; }

  //The main loop -
    //this is what iterates per-frame in your game. You generally want to do your animation based on fixed time steps, so for now we'll
    //just kind of not not worry about that.

    //Basically the main loop just runs until the user has told us that they want to quit - until that time, we'll continue to
    //loop through this process of handling the shit for the keyboard, then drawing the shit that we do in the draw function



    SDL_Delay(1000);








































  //GL WINDOW
    SDL_Init( SDL_INIT_VIDEO );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 5 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    static const int width = 1200;
    static const int height = 700;

    SDL_Window * window = SDL_CreateWindow( "OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
    SDL_GLContext context = SDL_GL_CreateContext( window );








    GLuint vs, fs, program;

    vs = glCreateShader( GL_VERTEX_SHADER );
    fs = glCreateShader( GL_FRAGMENT_SHADER );

    int length = strlen( vertex_shader );
    glShaderSource( vs, 1, ( const GLchar ** )&vertex_shader, &length );
    glCompileShader( vs );

    GLint status;
    glGetShaderiv( vs, GL_COMPILE_STATUS, &status );
    if( status == GL_FALSE )
    {
        fprintf( stderr, "vertex shader compilation failed\n" );
        return 1;
    }

    length = strlen( fragment_shader );
    glShaderSource( fs, 1, ( const GLchar ** )&fragment_shader, &length );
    glCompileShader( fs );

    glGetShaderiv( fs, GL_COMPILE_STATUS, &status );
    if( status == GL_FALSE )
    {
        fprintf( stderr, "fragment shader compilation failed\n" );
        return 1;
    }

    program = glCreateProgram();
    glAttachShader( program, vs );
    glAttachShader( program, fs );

    glBindAttribLocation( program, attrib_position, "i_position" );
    glBindAttribLocation( program, attrib_color, "i_color" );
    glLinkProgram( program );

    glUseProgram( program );

    glDisable( GL_DEPTH_TEST );
    glClearColor( 0.5, 0.0, 0.0, 0.0 );
    glViewport( 0, 0, width, height );

    GLuint vao, vbo;

    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );
    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    glEnableVertexAttribArray( attrib_position );
    glEnableVertexAttribArray( attrib_color );

    glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
    glVertexAttribPointer( attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )(4 * sizeof(float)) );

    const GLfloat g_vertex_buffer_data[] = {
    /*  R, G, B, A, X, Y  */
        1, 0, 0, 1, 0, 0,
        0, 1, 0, 1, width, 0,
        0, 0, 1, 1, width, height,

        1, 0, 0, 1, 0, 0,
        0, 0, 1, 1, width, height,
        1, 1, 1, 1, 0, height
    };

    glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), g_vertex_buffer_data, GL_STATIC_DRAW );

    t_mat4x4 projection_matrix;
    mat4x4_ortho( projection_matrix, 0.0f, (float)width, (float)height, 0.0f, 0.0f, 100.0f );
    glUniformMatrix4fv( glGetUniformLocation( program, "u_projection_matrix" ), 1, GL_FALSE, projection_matrix );




    //can we do images on top of OpenGL?  - looking like not really (12/18)

    // SDL_Renderer * ren  = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    // if (ren == NULL){ cerr << "SDL_CreateRenderer Error" << SDL_GetError() << endl; return EXIT_FAILURE; }
    //
    // SDL_Surface * bmp = SDL_LoadBMP("../2d/grumpy-cat.bmp");
    // if (bmp == NULL){ cerr << "SDL_LoadBMP Error: " << SDL_GetError() << endl; return EXIT_FAILURE; }
    //
    // SDL_Texture * tex = SDL_CreateTextureFromSurface(ren, bmp);
    // if (tex == NULL){ cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << endl; return EXIT_FAILURE; }


    // SDL_FreeSurface(bmp);
    //
    // SDL_Rect SrcRect; //where are we taking pixels from?
    // SDL_Rect DestRect;  //the pixels we took from SrcRect?
    //
    // DestRect.x = SrcRect.x = 0;
    // DestRect.y = SrcRect.y = 0;
    // DestRect.w = SrcRect.w = 620;
    // DestRect.h = SrcRect.h = 387;


    bool blink = true;

    //Main loop flags
    bool quit = false;

    while(!quit)  //main loop
    {
      SDL_RenderClear(ren); //clear our background

      if(SDL_GetTicks() < 5000)
      {

        SDL_Rect SrcRect; //where are we taking pixels from?
        SDL_Rect DestRect;  //the pixels we took from SrcRect?

        SrcRect.x = 0;
        SrcRect.y = 0;
        SrcRect.w = 720;
        SrcRect.h = 405;

        DestRect.x = 0;
        DestRect.y = 0;
        DestRect.w = 720;
        DestRect.h = 405;

        SDL_RenderCopy(ren, splash, &SrcRect, &DestRect);
        SDL_RenderPresent(ren); //swap buffers so that this most recently drawn material is shown to the user

        SDL_Delay(1200);

      }
      else
      {

        SDL_SetRenderDrawColor(ren,45,38,22,SDL_ALPHA_OPAQUE); //set the color you want to draw with

        SDL_Rect fullscreen;
        fullscreen.x = 0;
        fullscreen.y = 0;
        fullscreen.w = 720;
        fullscreen.h = 480;
        //put a base layer of that color down
        SDL_RenderFillRect(ren, &fullscreen);

        //set the new color to draw with
        SDL_SetRenderDrawColor(ren,45,45,45,SDL_ALPHA_OPAQUE); //set the color you want to draw with

        //learning how to chop up images with https://wiki.libsdl.org/SDL_RenderCopy
        //rectangles tell SDL where you want to read from/write from -

        //The implication here is that you have the opportunity to define where your pixels are going to show up on the screen, and what shape will they be.
        //Squashing/stretching can be achieved by changing the shape of that rectangle - look at the random generation of widths/heights to see examples.

        SDL_Rect SrcRect; //where are we taking pixels from?
        SDL_Rect DestRect;  //the pixels we took from SrcRect?

        //these random number generators give me some ability to randomly place the destination rectangles - you'll see
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> xdist(0,619);  //between zero and the width of the image
        std::uniform_int_distribution<int> sxdist(0,720);  //between zero and the width of the screen
        std::uniform_int_distribution<int> ydist(0,387);  //between zero and the height of the image
        std::uniform_int_distribution<int> sydist(0,480);  //between zero and the height of the screen
        std::uniform_int_distribution<int> wdist(16,128);  //some range of values
        std::uniform_int_distribution<int> hdist(25,175);  //some range of values

        //image dimensions are 620,387
        //I'm going to chop that into a 4x4 grid


        for(int x = 0; x < 4; x++)
        {
          for(int y = 0; y < 4; y++)
          {
            SrcRect.x = x*(620/4);    //here's some math to get x
            SrcRect.y = y*(387/4);   //here's some math to get y
            SrcRect.w = (620/4);    //width is just distance between two samples
            SrcRect.h = (387/4);   //similar situation for height

            if(aligned)
            { //'shrunk in' version of the cat face - this was kind of trial and error
              DestRect.x = x*(620/4)+80+10*cos(0.0004f*SDL_GetTicks());
              DestRect.y = y*(387/4)+70+4*sin(0.0004f*SDL_GetTicks());
              DestRect.w = (620/4)-60+40*cos(0.0004f*SDL_GetTicks());
              DestRect.h = (387/4)-40+10*sin(0.0004f*SDL_GetTicks());
            }
            else
            { //give me some random location to draw this shit at
              DestRect.x = xdist(mt);     //tell me where that shit's at on x
              DestRect.y = ydist(mt);    //tell me where that shit's at on y
              DestRect.w = wdist(mt);   //tell me how wide that shit is
              DestRect.h = hdist(mt);  //tell me how tall that shit is
            }

            for(int i = 0; i < 4500; i++)
            {
              SDL_RenderDrawPoint(ren, sxdist(mt), sydist(mt));
            }

            SDL_RenderCopy(ren, tex, &SrcRect, &DestRect);  //copy from SrcRect to DestRect
          }//end y
        }//end x

        SDL_RenderPresent(ren); //swap buffers so that this most recently drawn material is shown to the user

        // if(!aligned)
        //   SDL_Delay(1200); //wait some period of time so as not to cause as bad a seizure
      }


      glClear( GL_COLOR_BUFFER_BIT );

      SDL_Event event;
      while( SDL_PollEvent( &event ) )
      {
          switch( event.type )
          {
              case SDL_KEYUP:
                  if( event.key.keysym.sym == SDLK_ESCAPE )
                      return 0;
                  break;
          }
      }


      blink = !blink;
      cout << " SDL_GL_MakeCurrent returns " <<  SDL_GL_MakeCurrent( window, context) << " (0 is success)" << std::endl;

      glBindVertexArray( vao );

      // glUniform1i(glGetUniformLocation( shader_program, "mode" ), 0);


      if(blink)
        glDrawArrays( GL_TRIANGLES, 0, 6 );
      else
        glClear( GL_COLOR_BUFFER_BIT );


      // SDL_RenderCopy(ren, tex, &SrcRect, &DestRect);
      // SDL_RenderPresent(ren);

      SDL_GL_SwapWindow( window );
      SDL_Delay( 100 );

    }




    SDL_GL_DeleteContext( context );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}
