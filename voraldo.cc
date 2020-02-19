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
  SDL_Init( SDL_INIT_EVERYTHING );

  current_menu_state = MAIN_MENU; //initial state of the menu

  cout << endl << endl << "info dump:" << endl;
  collect_startup_info();

  cout << "populating menu options" << endl;
  assemble_menu_layouts();

  cout << "setting up ttf font rendering" << endl;
  sdl_ttf_init();

  cout << "creating GL window" << endl;
  create_gl_window();

  cout << "creating info window" << endl;
  create_info_window();

  cout << "initializing voraldo components" << endl;
  //todo

  cout << "entering main loop" << endl;

  //DEBUG ENABLE
  glEnable              ( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( MessageCallback, 0 );

  while(current_menu_state != EXIT)
  {
    SDL_RenderClear(SDL_2D_renderer); //clear our background

    draw_menu();    //do this first, so that we don't draw the wrong menu when we just entered the EXIT state
    take_input();   //this is where any state changes are going to happen

    SDL_RenderPresent(SDL_2D_renderer); //swap buffers

    SDL_Delay(10);
  }

  cout << "dropping out of main loop" << endl;

}

voraldo::~voraldo()
{
  SDL_GL_DeleteContext( GLcontext );
  SDL_DestroyWindow( OpenGL_window );

  exit_splashBMP = SDL_LoadBMP(exit_splash_path.c_str());
  exit_splash = SDL_CreateTextureFromSurface(SDL_2D_renderer, exit_splashBMP);

  SDL_FreeSurface(exit_splashBMP); //free that surface
  SDL_DestroyRenderer(SDL_2D_renderer);

  TTF_CloseFont( font );

  SDL_DestroyWindow( Informational_window );
  SDL_Quit();

  cout << "GOODBYE" << endl << endl << endl; //last line of code before main's return statement
}

void voraldo::take_input()
{
  // //in order to handle buttons, we're going to check the event's windowid,
  // //event.button.windowID against the Informational_window id, which is acquired with
  //
  // //   Uint32 SDL_GetWindowID(SDL_Window* window)
  //
  ////might want to mess with this eventually, to grab focus
  //
  //   // SDL_ShowWindow(Informational_window);
  //   // SDL_ShowWindow(OpenGL_window);
  //
  //   // SDL_RaiseWindow(Informational_window);
  //   // SDL_RaiseWindow(OpenGL_window);

  SDL_Event event;
  while( SDL_PollEvent( &event ) )
  {
      switch( event.type )
      {
        case SDL_KEYUP:
          //handle input from keyboard
          switch( event.key.keysym.sym )
          {
            case SDLK_ESCAPE:
              current_menu_state = EXIT;
              break;

            case SDLK_UP:

              break;



            case SDLK_1:
              current_menu_state = MAIN_MENU;
              break;


            case SDLK_2:

              current_menu_state = DRAW_MENU;
              break;


            case SDLK_3:

              current_menu_state = MASK_MENU;
              break;


            case SDLK_4:
              current_menu_state = UTIL_MENU;

              break;

            case SDLK_5:
              current_menu_state = SPHERE_CONFIG;
              break;
          }

          break;

        //mouse events

        //other events

        // ...

        default:
          break;
      }
  }
}

void voraldo::collect_startup_info()
{


  //this is how you query the screen resolution
  SDL_DisplayMode dm;
  SDL_GetDesktopDisplayMode(0, &dm);

  //pulling these out because I'm going to try to span the whole screen with
  //the windows, without overlaps if possible, in a way that's flexible
  total_screen_width = dm.w;
  total_screen_height = dm.h;

  std::stringstream m;


  //PATH
  cout << " Program launched from: " << SDL_GetBasePath() << endl;
  m << " Program launched from: " << SDL_GetBasePath() << endl;

  startup_info.push_back(m.str());  //get the path on the info vector
  std::stringstream().swap(m);     //swap m with a default constructed stringstream to clear

  //PLATFORM
  cout << " Running on " << std::string(SDL_GetPlatform()) << endl;
  m << " Running on " << std::string(SDL_GetPlatform()) << endl;

  startup_info.push_back(m.str());  //get the path on the info vector
  std::stringstream().swap(m);     //swap m with a default constructed stringstream to clear

  //SCREEN RESOLUTION
  cout << " Screen resolution is: " << total_screen_width << " by " << total_screen_height << endl;
  m << " Screen resolution is: " << total_screen_width << " by " << total_screen_height << endl;

  startup_info.push_back(m.str());  //get the path on the info vector
  std::stringstream().swap(m);     //swap m with a default constructed stringstream to clear


  //LOGICAL CPUs (THREADS)
  cout << " Logical CPU cores: " << SDL_GetCPUCount() << endl;
  m << " Logical CPU cores: " << SDL_GetCPUCount() << endl;

  startup_info.push_back(m.str());  //get the path on the info vector
  std::stringstream().swap(m);     //swap m with a default constructed stringstream to clear


  //SYSTEM RAM
  cout << " System RAM: " << SDL_GetSystemRAM() << " MB" << endl;
  m << " System RAM: " << SDL_GetSystemRAM() << " MB" << endl;

  startup_info.push_back(m.str());  //get the path on the info vector
  std::stringstream().swap(m);     //swap m with a default constructed stringstream to clear

  //BATTERY INFO
  int secs, pct;
  switch (SDL_GetPowerInfo(&secs, &pct))
  {
    case SDL_POWERSTATE_UNKNOWN:
      cout << " Battery in undetermined state" << endl;
      m << " Battery in undetermined state" << endl;
      break;

    case SDL_POWERSTATE_ON_BATTERY:
      cout << " Operating on battery - " << pct << " percent remaining, estimated " << secs << "seconds" << endl;
      m << " Operating on battery - " << pct << " percent remaining, estimated " << secs << "seconds" << endl;
      break;

    case SDL_POWERSTATE_NO_BATTERY:
      cout << " No Battery Present" << endl;
      m << " No Battery Present" << endl;
      break;

    case SDL_POWERSTATE_CHARGING:
      cout << " Battery is charging - " << pct << " percent" << endl;
      m << " Battery is charging - " << pct << " percent" << endl;
      break;

    case SDL_POWERSTATE_CHARGED:
      cout << " Battery is fully charged" << endl;
      m << " Battery is fully charged" << endl;
      break;
  }

  startup_info.push_back(m.str());  //get the path on the info vector
  std::stringstream().swap(m);     //swap m with a default constructed stringstream to clear

  cout << endl << endl;

}

void voraldo::assemble_menu_layouts()
{
  //this function populates 'menu'
  menu.resize(NUM_STATES);

  menu_state ms;
  menu_layout ml;

  // //overarching structure for the menu layout - labels and links
  // typedef struct menu_layout_t
  // {
  //   std::vector<std::string> labels;  //zero index is heading for this menu_state
  //   std::vector<menu_state> links;    //zero index of links is parent
  // } menu_layout;

  for (int i = EXIT; i != INVALID; i++)
  {
    ms = static_cast<menu_state>(i);  //workaround to make iterating over an enum work
    ml = menu_layout();              //reinitialize to zero out the vectors

    switch(ms)
    {
      case EXIT:
        //don't need any labels or links for this
        break;


      case MAIN_MENU:
        ml.labels.push_back("MAIN MENU");
        ml.links.push_back(EXIT);

        ml.labels.push_back("  1: DRAW MENU");
        ml.links.push_back(DRAW_MENU);

        ml.labels.push_back("  2: MASK MENU");
        ml.links.push_back(MASK_MENU);

        ml.labels.push_back("  3: LIGHT MENU");
        ml.links.push_back(LIGHT_MENU);

        ml.labels.push_back("  4: CA MENU");
        ml.links.push_back(CA_MENU);

        ml.labels.push_back("  5: UTILITIES MENU");
        ml.links.push_back(UTIL_MENU);

        break;


      case DRAW_MENU:
        ml.labels.push_back("DRAW MENU");
        ml.links.push_back(MAIN_MENU);

        ml.labels.push_back("  1: SPHERE CONFIG MENU");
        ml.links.push_back(SPHERE_CONFIG);
        break;


      case MASK_MENU:
        ml.labels.push_back("MASK MENU");
        ml.links.push_back(MAIN_MENU);
        break;


      case LIGHT_MENU:
        ml.labels.push_back("LIGHTING MENU");
        ml.links.push_back(MAIN_MENU);
        break;


      case CA_MENU:
        ml.labels.push_back("CELLULAR AUTOMATA MENU");
        ml.links.push_back(MAIN_MENU);
        break;


      case UTIL_MENU:
        ml.labels.push_back("UTILITIES MENU");
        ml.links.push_back(MAIN_MENU);
        break;


      case SPHERE_CONFIG:
        ml.links.push_back(DRAW_MENU);
        ml.labels.push_back("SPHERE CONFIG MENU");


        break;

      default:
        break;
    }

    menu[ms] = ml; //put it in the thing so we can get at it later

  }
}

void voraldo::draw_menu()
{
  SDL_Rect s;
  int basex, basey;



//DRAW THE MAIN MENU BOX
  if (current_menu_state == MAIN_MENU)
  {//highlighted color
    SDL_SetRenderDrawColor(SDL_2D_renderer, 128, 125, 110, 255);
    basex = 9;
    basey = 5;
  }
  else if(current_menu_state == MASK_MENU || current_menu_state == DRAW_MENU || current_menu_state == UTIL_MENU || current_menu_state == LIGHT_MENU || current_menu_state == CA_MENU)
  {
    //somewhat dimmed
    SDL_SetRenderDrawColor(SDL_2D_renderer, 100, 100, 100, 255);
    basex = 14;
    basey = 22;
  }
  else
  {
    //somewhat more dimmed
    SDL_SetRenderDrawColor(SDL_2D_renderer, 75, 75, 75, 255);
    basex = 19;
    basey = 39;
  }

  //draw the main menu box with whatever color is set
  s = {5,5,Infowindowwidth - 30,Infowindowheight - 50};
  SDL_RenderFillRect(SDL_2D_renderer, &s);

  //border is the same whatever the case
  SDL_SetRenderDrawColor(SDL_2D_renderer, 50, 50, 50, 255);
  SDL_RenderDrawRect(SDL_2D_renderer, &s);






  //stuff for the submenus
  if(current_menu_state != MAIN_MENU)
  {
    s = {10,22,Infowindowwidth - 30,Infowindowheight - 50};


    if(current_menu_state == MASK_MENU || current_menu_state == DRAW_MENU || current_menu_state == UTIL_MENU || current_menu_state == LIGHT_MENU || current_menu_state == CA_MENU)
    {
      SDL_SetRenderDrawColor(SDL_2D_renderer, 128, 125, 110, 255);
    }
    else
    { //this is called if a submenu of one of the big three is active (mask,main, or util are in bg)
      SDL_SetRenderDrawColor(SDL_2D_renderer, 100, 100, 100, 255);
    }

    //draw with whatever color is set
    SDL_RenderFillRect(SDL_2D_renderer, &s);
    SDL_SetRenderDrawColor(SDL_2D_renderer, 50, 50, 50, 255);
    SDL_RenderDrawRect(SDL_2D_renderer, &s);

    //if it's not level two, it's level three
    if(!(current_menu_state == MASK_MENU || current_menu_state == DRAW_MENU || current_menu_state == UTIL_MENU || current_menu_state == LIGHT_MENU || current_menu_state == CA_MENU))
    {
      s = {15,39,Infowindowwidth - 30,Infowindowheight - 50};

      SDL_SetRenderDrawColor(SDL_2D_renderer, 128, 125, 110, 255);
      SDL_RenderFillRect(SDL_2D_renderer, &s);
      SDL_SetRenderDrawColor(SDL_2D_renderer, 50, 50, 50, 255);
      SDL_RenderDrawRect(SDL_2D_renderer, &s);


      //this may be the spot to handle the second level menu,
      //with a switch that looks at the links[0] to see what the parent is
      switch (menu[current_menu_state].links[0])
      {
        case DRAW_MENU:
          ttf_string("DRAW MENU", 14, 22, 0, 0, 0);
          break;

        case MASK_MENU:
          ttf_string("MASK MENU", 14, 22, 0, 0, 0);
          break;

        case UTIL_MENU:
          ttf_string("UTILITIES MENU", 14, 22, 0, 0, 0);
          break;

        case LIGHT_MENU:
          ttf_string("LIGHTING MENU", 14, 22, 0, 0, 0);
          break;

        case CA_MENU:
          ttf_string("CELLULAR AUTOMATA MENU", 14, 22, 0, 0, 0);
          break;

      }
    }
  }
  //this top bit will always be the main menu - so the label can be static
  ttf_string("MAIN MENU", 9, 5, 0, 0, 0);

  for(int i = 0; i < menu[current_menu_state].labels.size(); i++)
  {
    ttf_string(menu[current_menu_state].labels[i], basex, basey+(20*i), 0, 0, 0);
  }

}

void voraldo::create_gl_window()
{
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

  windowwidth = 2*(total_screen_width/3);
  windowheight = total_screen_height;

  OpenGL_window = SDL_CreateWindow( "OpenGL Window", total_screen_width/3, 0, windowwidth, windowheight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS );
  GLcontext = SDL_GL_CreateContext( OpenGL_window );


  SDL_GL_MakeCurrent(OpenGL_window, GLcontext);

  glClearColor( 0.26, 0.16, 0.0, 1.0 );
  glClear( GL_COLOR_BUFFER_BIT );
  SDL_GL_SwapWindow( OpenGL_window );


  // SDL_Delay(10);
}

void voraldo::create_info_window()
{
  Infowindowwidth = total_screen_width/3;
  Infowindowheight = total_screen_height;

  Informational_window = SDL_CreateWindow("Voraldo", 0, 0, Infowindowwidth, Infowindowheight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS );
  SDL_2D_renderer = SDL_CreateRenderer(Informational_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  SDL_SetRenderDrawColor(SDL_2D_renderer, 128, 125, 110, 255);
  SDL_RenderClear(SDL_2D_renderer); //clear our background

  SDL_Rect SrcRect  = {0,0,1200,400};   //where are we taking pixels from when we load the splash screen
  SDL_Rect DestRect = {0,0,Infowindowwidth,Infowindowwidth/3};  //where are we putting the pixels we took from SrcRect?

  splashBMP = SDL_LoadBMP(splash_path.c_str());
  splash = SDL_CreateTextureFromSurface(SDL_2D_renderer, splashBMP);
  SDL_FreeSurface(splashBMP);

  SDL_RenderCopy(SDL_2D_renderer, splash, &SrcRect, &DestRect);

  int baseoffset = Infowindowwidth/3;

  ttf_string(std::string("SYSTEM INFO:"), 10, baseoffset, 240, 190, 0);
  ttf_string(startup_info[0], 10, baseoffset+20, 200, 170, 0);
  ttf_string(startup_info[1], 10, baseoffset+40, 200, 170, 0);
  ttf_string(startup_info[2], 10, baseoffset+60, 200, 170, 0);
  ttf_string(startup_info[3], 10, baseoffset+80, 200, 170, 0);
  ttf_string(startup_info[4], 10, baseoffset+100, 200, 170, 0);
  ttf_string(startup_info[5], 10, baseoffset+120, 200, 170, 0);


  ttf_string(std::string("INSTRUCTIONS:"), 10, baseoffset+240, 240, 190, 0);

  ttf_string(std::string("  hit the number of the menu option you wish to select"), 10, baseoffset+270, 240, 190, 0);
  ttf_string(std::string("  hit escape to go up a level"), 10, baseoffset+290, 240, 190, 0);
  ttf_string(std::string("  hit escape on the main menu to exit"), 10, baseoffset+310, 240, 190, 0);




  ttf_string(std::string("HIT ENTER TO CONTINUE or ESCAPE TO EXIT..."), 10, baseoffset+440, 240, 190, 0);

  SDL_RenderPresent(SDL_2D_renderer); //swap buffers


  SDL_Event event;

  while(1)
  {
    SDL_PollEvent(&event);

    if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_RETURN)
      break;

    if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
    {
      current_menu_state = EXIT;
      break;
    }
  }



}

void voraldo::sdl_ttf_init()
{
  //initialize the text engine, load a local .ttf file, report error if neccesary
  if( TTF_Init() == -1 )  cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
  font = TTF_OpenFont( "resources/fonts/Braciola MS.ttf", 14 );
  // font = TTF_OpenFont( "resources/fonts/SquareDotDigital7-Dpv9.ttf", 20 );

  if(font == NULL) cout << "loading failed" << endl;
}

void voraldo::font_test()
{
  for(int i = 5; i < (2*(total_screen_height/3)+20); i+= 18)
  {
    SDL_Rect s = {5, i, 200, 16};
    SDL_SetRenderDrawColor(SDL_2D_renderer, 120, 33, 44, 255);
    SDL_RenderFillRect(SDL_2D_renderer, &s);
    ttf_string("FONT TEST font test", 10, i, 206, 143, 0);
  }
}

void voraldo::ttf_string(std::string s, int basex, int basey, unsigned char r, unsigned char g, unsigned char b)
{
  SDL_Color col = {r, g, b};
  SDL_Surface * message_surface = TTF_RenderText_Solid( font, s.c_str(), col );
  SDL_Texture * message_text = SDL_CreateTextureFromSurface( SDL_2D_renderer, message_surface );

  int wid = message_surface->w;
  int hei = message_surface->h;

  SDL_Rect renderQuad = { basex, basey, wid, hei };
  SDL_RenderCopy( SDL_2D_renderer, message_text, NULL, &renderQuad );
  SDL_FreeSurface( message_surface );
  SDL_DestroyTexture( message_text );

  //for the little animation dealio
  // SDL_RenderPresent(SDL_2D_renderer); //swap buffers
  // SDL_Delay(45);
}
