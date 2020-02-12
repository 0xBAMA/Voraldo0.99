#include "voraldo.h"

//DEBUG STUFF

void GLAPIENTRY MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  bool show_high_severity         = true;
  if(severity == GL_DEBUG_SEVERITY_HIGH && show_high_severity)
    fprintf( stderr, "        GL CALLBACK: %s type = 0x%x, severity = GL_DEBUG_SEVERITY_HIGH, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, message );

  bool show_medium_severity       = true;
  if(severity == GL_DEBUG_SEVERITY_MEDIUM && show_medium_severity)
    fprintf( stderr, "        GL CALLBACK: %s type = 0x%x, severity = GL_DEBUG_SEVERITY_MEDIUM, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, message );

  bool show_low_severity          = true;
  if(severity == GL_DEBUG_SEVERITY_LOW && show_low_severity)
    fprintf( stderr, "        GL CALLBACK: %s type = 0x%x, severity = GL_DEBUG_SEVERITY_LOW, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, message );

  bool show_notification_severity = true;
  if(severity == GL_DEBUG_SEVERITY_NOTIFICATION && show_notification_severity)
    fprintf( stderr, "        GL CALLBACK: %s type = 0x%x, severity = GL_DEBUG_SEVERITY_NOTIFICATION, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, message );
}


// Initialize with the reference values for the permutation vector
PerlinNoise::PerlinNoise() {

	// Initialize the permutation vector with the reference values
	p = {
		151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
		8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
		35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,
		134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
		55,46,245,40,244,102,143,54, 65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
		18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,
		250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
		189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167,
		43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,
		97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,
		107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };
	// Duplicate the permutation vector
	p.insert( p.end( ), p.begin( ), p.end( ) );
}

// Generate a new permutation vector based on the value of seed
PerlinNoise::PerlinNoise( unsigned int seed )
{
	p.resize( 256 );

	// Fill p with values from 0 to 255
	std::iota( p.begin( ), p.end( ), 0 );

	// Initialize a random engine with seed
	std::default_random_engine engine( seed );

	// Suffle  using the above random engine
	std::shuffle( p.begin( ), p.end( ), engine );

	// Duplicate the permutation vector
	p.insert( p.end( ), p.begin( ), p.end( ) );
}

double PerlinNoise::noise( double x, double y, double z ) {
	// Find the unit cube that contains the point
	int X = (int) floor(x) & 255;
	int Y = (int) floor(y) & 255;
	int Z = (int) floor(z) & 255;

	// Find relative x, y,z of point in cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute fade curves for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = p[X] + Y;
	int AA = p[A] + Z;
	int AB = p[A + 1] + Z;
	int B = p[X + 1] + Y;
	int BA = p[B] + Z;
	int BB = p[B + 1] + Z;

	// Add blended results from 8 corners of cube
	double res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x-1, y, z)), lerp(u, grad(p[AB], x, y-1, z), grad(p[BB], x-1, y-1, z))),	lerp(v, lerp(u, grad(p[AA+1], x, y, z-1), grad(p[BA+1], x-1, y, z-1)), lerp(u, grad(p[AB+1], x, y-1, z-1),	grad(p[BB+1], x-1, y-1, z-1))));
	return (res + 1.0)/2.0;
}

double PerlinNoise::fade(double t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b) {
	return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y, double z) {
	int h = hash & 15;
	// Convert lower 4 bits of hash into 12 gradient directions
	double u = h < 8 ? x : y,
		   v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}



voraldo::voraldo()
{
  SrcRect  = {0,0,720,405};
  DestRect = {0,0,720,405};

  cout << "creating GL window" << endl;
  create_gl_window();
  cout << "creating info window" << endl;
  create_info_window();

  cout << "initializing voraldo components" << endl;
  //todo

  cout << "entering main loop" << endl;
  while(1)
  {
    if(!main_loop())
    {
      break;
    }
  }
}



int voraldo::main_loop()
{
  //in order to handle buttons, we're going to check the event's windowid,
  //event.button.windowID against the Informational_window id, which is acquired with

  //   Uint32 SDL_GetWindowID(SDL_Window* window)


  SDL_Event event;
  while( SDL_PollEvent( &event ) )
  {
      switch( event.type )
      {
          case SDL_KEYUP:
              if( event.key.keysym.sym == SDLK_ESCAPE )
              {
                cout << "GOODBYE" << endl;
                return 0;
              }
              break;

          default:
            return 1;
            break;
      }
  }
}

voraldo::~voraldo()
{
  SDL_GL_DeleteContext( GLcontext );
  SDL_DestroyWindow( OpenGL_window );

  SDL_Delay(30);


  SDL_Renderer* renderer = SDL_CreateRenderer(Informational_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  SDL_Surface* exit_splashBMP = SDL_LoadBMP("resources/exit_splash.bmp");
  if (exit_splashBMP == NULL)  cerr << "SDL_LoadBMP Error: " << SDL_GetError() << endl;

  SDL_Texture* exit_splash = SDL_CreateTextureFromSurface(renderer, exit_splashBMP);
  SDL_FreeSurface(exit_splashBMP); //free that surface
  if (exit_splash == NULL) cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << endl;

  SDL_RenderClear(renderer); //clear our background

  SDL_RenderCopy(renderer, exit_splash, &SrcRect, &DestRect);  //blit the image to the window
  SDL_RenderPresent(renderer); //swap buffers so that this most recently drawn material is shown to the user
  SDL_DestroyRenderer(renderer);

  SDL_Delay(1200);  //hold for some period of time to show the exit splash

  SDL_DestroyWindow( Informational_window );
  SDL_Quit();
}


void voraldo::create_gl_window()
{
  SDL_Init( SDL_INIT_EVERYTHING );
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

  OpenGL_window = SDL_CreateWindow( "OpenGL Window", 200, 0, windowwidth, windowheight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
  GLcontext = SDL_GL_CreateContext( OpenGL_window );

  //DEBUG
  glEnable              ( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( MessageCallback, 0 );

  glClearColor( 0.6, 0.16, 0.0, 1.0 );

  SDL_Delay(10);
}

void voraldo::create_info_window()
{
  Informational_window = SDL_CreateWindow("Hello World!", 10, 100, 720, 405, SDL_WINDOW_OPENGL);
  if (Informational_window == NULL) cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;

  SDL_Renderer* renderer = SDL_CreateRenderer(Informational_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) cerr << "SDL_CreateRenderer Error" << SDL_GetError() << endl;

  SDL_Surface* splashBMP = SDL_LoadBMP("resources/splash.bmp");
  if (splashBMP == NULL) cerr << "SDL_LoadBMP Error: " << SDL_GetError() << endl;

  SDL_Texture* splash = SDL_CreateTextureFromSurface(renderer, splashBMP);  SDL_FreeSurface(splashBMP);
  if (splash == NULL) cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << endl;

  SDL_RenderCopy(renderer, splash, &SrcRect, &DestRect);
  SDL_RenderPresent(renderer); //swap buffers so that this most recently drawn material is shown to the user

  SDL_Delay(2000);

  SDL_RenderClear(renderer); //clear our background




  //write some text
  if( TTF_Init() == -1 )  cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;

  TTF_Font *font = TTF_OpenFont( "resources/fonts/Braciola MS.ttf", 12 );

  if(font == NULL)  cout << "loading failed" << endl;

  SDL_Color clrFg = {0,0,255};  // Blue ("Fg" is foreground)

  std::string blep("This is some example text.\n");

  SDL_Surface *sText = TTF_RenderText_Solid( font, blep.c_str(), clrFg );

  int wid = sText->w;
  int hei = sText->h;

  SDL_Texture* message = SDL_CreateTextureFromSurface( renderer, sText );
  SDL_FreeSurface( sText );

  SDL_Rect renderQuad = { 0, 0, wid, hei };
  SDL_RenderCopy( renderer, message, NULL, &renderQuad );

  //Free font
  TTF_CloseFont( font );
  font = NULL;



  SDL_RenderPresent(renderer); //swap buffers
  SDL_Delay(1000);
  SDL_DestroyRenderer(renderer);
}
