/* 
 * Tux Racer 
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "tuxracer.h"
#include "winsys.h"
#include "audio.h"

/* Windowing System Abstraction Layer */
/* Abstracts creation of windows, handling of events, etc. */

#if defined( HAVE_SDL )

#if defined( HAVE_SDL_MIXER )
#   include "SDL_mixer.h"
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* SDL version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static SDL_Window *window = NULL;
static SDL_GLContext gl_context = NULL;

/* SDL2 port: translate an SDL2 keycode into the key numbering used by the
   game (ASCII below 256, WSK_* special keys at 256 and above). */
static unsigned int translate_key( SDL_Keycode sym )
{
    if ( sym > 0 && sym < 256 ) {
	return (unsigned int) sym;
    }
    switch ( sym ) {
    case SDLK_KP_0: return WSK_KP0;
    case SDLK_KP_1: return WSK_KP1;
    case SDLK_KP_2: return WSK_KP2;
    case SDLK_KP_3: return WSK_KP3;
    case SDLK_KP_4: return WSK_KP4;
    case SDLK_KP_5: return WSK_KP5;
    case SDLK_KP_6: return WSK_KP6;
    case SDLK_KP_7: return WSK_KP7;
    case SDLK_KP_8: return WSK_KP8;
    case SDLK_KP_9: return WSK_KP9;
    case SDLK_KP_PERIOD: return WSK_KP_PERIOD;
    case SDLK_KP_DIVIDE: return WSK_KP_DIVIDE;
    case SDLK_KP_MULTIPLY: return WSK_KP_MULTIPLY;
    case SDLK_KP_MINUS: return WSK_KP_MINUS;
    case SDLK_KP_PLUS: return WSK_KP_PLUS;
    case SDLK_KP_ENTER: return WSK_KP_ENTER;
    case SDLK_KP_EQUALS: return WSK_KP_EQUALS;
    case SDLK_UP: return WSK_UP;
    case SDLK_DOWN: return WSK_DOWN;
    case SDLK_RIGHT: return WSK_RIGHT;
    case SDLK_LEFT: return WSK_LEFT;
    case SDLK_INSERT: return WSK_INSERT;
    case SDLK_HOME: return WSK_HOME;
    case SDLK_END: return WSK_END;
    case SDLK_PAGEUP: return WSK_PAGEUP;
    case SDLK_PAGEDOWN: return WSK_PAGEDOWN;
    case SDLK_F1: return WSK_F1;
    case SDLK_F2: return WSK_F2;
    case SDLK_F3: return WSK_F3;
    case SDLK_F4: return WSK_F4;
    case SDLK_F5: return WSK_F5;
    case SDLK_F6: return WSK_F6;
    case SDLK_F7: return WSK_F7;
    case SDLK_F8: return WSK_F8;
    case SDLK_F9: return WSK_F9;
    case SDLK_F10: return WSK_F10;
    case SDLK_F11: return WSK_F11;
    case SDLK_F12: return WSK_F12;
    case SDLK_F13: return WSK_F13;
    case SDLK_F14: return WSK_F14;
    case SDLK_F15: return WSK_F15;
    case SDLK_NUMLOCKCLEAR: return WSK_NUMLOCK;
    case SDLK_CAPSLOCK: return WSK_CAPSLOCK;
    case SDLK_SCROLLLOCK: return WSK_SCROLLOCK;
    case SDLK_RSHIFT: return WSK_RSHIFT;
    case SDLK_LSHIFT: return WSK_LSHIFT;
    case SDLK_RCTRL: return WSK_RCTRL;
    case SDLK_LCTRL: return WSK_LCTRL;
    case SDLK_RALT: return WSK_RALT;
    case SDLK_LALT: return WSK_LALT;
    case SDLK_RGUI: return WSK_RMETA;
    case SDLK_LGUI: return WSK_LMETA;
    default: return WSK_NOT_AVAIL;
    }
}

static winsys_display_func_t display_func = NULL;
static winsys_idle_func_t idle_func = NULL;
static winsys_reshape_func_t reshape_func = NULL;
static winsys_keyboard_func_t keyboard_func = NULL;
static winsys_mouse_func_t mouse_func = NULL;
static winsys_motion_func_t motion_func = NULL;
static winsys_motion_func_t passive_motion_func = NULL;
static winsys_atexit_func_t atexit_func = NULL;

static bool_t redisplay = False;
static bool_t key_repeat_enabled = False;


/*---------------------------------------------------------------------------*/
/*! 
  Requests that the screen be redrawn
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_post_redisplay() 
{
    redisplay = True;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the display callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_display_func( winsys_display_func_t func )
{
    display_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the idle callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_idle_func( winsys_idle_func_t func )
{
    idle_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the reshape callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    reshape_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the keyboard callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse button-press callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    mouse_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse motion callback (when a mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_motion_func( winsys_motion_func_t func )
{
    motion_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse motion callback (when no mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    passive_motion_func = func;
}



/*---------------------------------------------------------------------------*/
/*! 
  Copies the OpenGL back buffer to the front buffer
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_swap_buffers()
{
    SDL_GL_SwapWindow( window );
}


/*---------------------------------------------------------------------------*/
/*! 
  Moves the mouse pointer to (x,y)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_warp_pointer( int x, int y )
{
    SDL_WarpMouseInWindow( window, x, y );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets up the SDL OpenGL rendering context
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
static void setup_sdl_video_mode()
{
    Uint32 window_flags = SDL_WINDOW_OPENGL;
    int width, height;

    /* Colour depth (bpp_mode) is chosen by the desktop under SDL2 */
    if ( getparam_fullscreen() ) {
	window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else {
	window_flags |= SDL_WINDOW_RESIZABLE;
    }

    width = getparam_x_resolution();
    height = getparam_y_resolution();

    window = SDL_CreateWindow( "Tux Racer",
			       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			       width, height, window_flags );
    if ( window == NULL ) {
	handle_system_error( 1, "Couldn't initialize video: %s", 
			     SDL_GetError() );
    }

    gl_context = SDL_GL_CreateContext( window );
    if ( gl_context == NULL ) {
	handle_system_error( 1, "Couldn't create OpenGL context: %s", 
			     SDL_GetError() );
    }

    /* Synchronise with the display refresh rate if possible */
    SDL_GL_SetSwapInterval( 1 );

    /* In fullscreen the real size is the desktop size */
    SDL_GetWindowSize( window, &width, &height );
    setparam_x_resolution( width );
    setparam_y_resolution( height );
}


/*---------------------------------------------------------------------------*/
/*! 
  Initializes the OpenGL rendering context, and creates a window (or 
  sets up fullscreen mode if selected)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_init( int *argc, char **argv, char *window_title, 
		  char *icon_title )
{
    Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;

    /*
     * Initialize SDL
     */
    if ( SDL_Init( sdl_flags ) < 0 ) {
	handle_error( 1, "Couldn't initialize SDL: %s", SDL_GetError() );
    }


    /* 
     * Init video 
     */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

#if defined( USE_STENCIL_BUFFER )
    /* Not sure if this is sufficient to activate stencil buffer  */
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
#endif

    setup_sdl_video_mode();

    SDL_SetWindowTitle( window, window_title );

}


/*---------------------------------------------------------------------------*/
/*! 
  Deallocates resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_shutdown()
{
    if ( gl_context ) {
	SDL_GL_DeleteContext( gl_context );
	gl_context = NULL;
    }
    if ( window ) {
	SDL_DestroyWindow( window );
	window = NULL;
    }
    SDL_Quit();
}

/*---------------------------------------------------------------------------*/
/*! 
  Enables/disables key repeat messages from being generated
  \return  
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_enable_key_repeat( bool_t enabled )
{
    /* SDL2 always generates repeats; they are filtered out in
       winsys_process_events() when disabled */
    key_repeat_enabled = enabled;
}


/*---------------------------------------------------------------------------*/
/*! 
  Shows/hides mouse cursor
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_show_cursor( bool_t visible )
{
    SDL_ShowCursor( visible );
}

/*---------------------------------------------------------------------------*/
/*! 
  Processes and dispatches events.  This function never returns.
  \return  No.
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_process_events()
{
    SDL_Event event; 
    unsigned int key;
    int x, y;

    while (True) {

	SDL_LockAudio();
	SDL_UnlockAudio();

	while ( SDL_PollEvent( &event ) ) {
	    
	    switch ( event.type ) {
	    case SDL_QUIT:
		winsys_exit( 0 );
		break;

	    case SDL_KEYDOWN:
		if ( event.key.repeat && !key_repeat_enabled ) {
		    break;
		}
		/* Alt+Enter toggles fullscreen */
		if ( ( event.key.keysym.sym == SDLK_RETURN ) &&
		     ( event.key.keysym.mod & KMOD_ALT ) ) 
		{
		    bool_t fs = !getparam_fullscreen();
		    setparam_fullscreen( fs );
		    SDL_SetWindowFullscreen( window, fs ? 
					     SDL_WINDOW_FULLSCREEN_DESKTOP : 0 );
		    break;
		}
		key = translate_key( event.key.keysym.sym );
		if ( keyboard_func && key != WSK_NOT_AVAIL ) {
		    SDL_GetMouseState( &x, &y );
		    (*keyboard_func)( key,
				      key >= 256,
				      False,
				      x, y );
		}
		break;

	    case SDL_KEYUP:
		key = translate_key( event.key.keysym.sym );
		if ( keyboard_func && key != WSK_NOT_AVAIL ) {
		    SDL_GetMouseState( &x, &y );
		    (*keyboard_func)( key,
				      key >= 256,
				      True,
				      x, y );
		}
		break;

	    case SDL_MOUSEBUTTONDOWN:
	    case SDL_MOUSEBUTTONUP:
		if ( mouse_func ) {
		    (*mouse_func)( event.button.button,
				   event.button.state,
				   event.button.x,
				   event.button.y );
		}
		break;

	    case SDL_MOUSEMOTION:
		if ( event.motion.state ) {
		    /* buttons are down */
		    if ( motion_func ) {
			(*motion_func)( event.motion.x,
					event.motion.y );
		    }
		} else {
		    /* no buttons are down */
		    if ( passive_motion_func ) {
			(*passive_motion_func)( event.motion.x,
						event.motion.y );
		    }
		}
		break;

	    case SDL_WINDOWEVENT:
		if ( event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED &&
		     reshape_func ) 
		{
		    (*reshape_func)( event.window.data1,
				     event.window.data2 );
		}

		break;
	    }

	    SDL_LockAudio();
	    SDL_UnlockAudio();
	}

#ifdef TR_AUTOTEST
	/* Test-only build (-DTR_AUTOTEST): presses Enter TR_AUTOKEY times,
	   2.5 s apart, saving a screenshot in TR_SHOTDIR before each press */
	{
	    static Uint32 next = 3000; static int n = 0;
	    if ( getenv( "TR_AUTOKEY" ) && n < atoi( getenv( "TR_AUTOKEY" ) ) &&
		 SDL_GetTicks() > next ) 
	    {
		extern char* take_screenshot( char* );
		char f[1024];
		next = SDL_GetTicks() + 2500; n++;
		if ( getenv( "TR_SHOTDIR" ) ) {
		    snprintf( f, sizeof f, "%s/shot%d.ppm", getenv( "TR_SHOTDIR" ), n );
		    take_screenshot( f );
		}
		if ( keyboard_func ) {
		    (*keyboard_func)( 13, False, False, 0, 0 );
		    (*keyboard_func)( 13, False, True, 0, 0 );
		}
	    }
	}
#endif
	if ( redisplay && display_func ) {
	    redisplay = False;
	    (*display_func)();
	} else if ( idle_func ) {
	    (*idle_func)();
	}

	/* Delay for 1 ms.  This allows the other threads to do some
	   work (otherwise the audio thread gets starved). */
	SDL_Delay(1);

    }

    /* Never exits */
    code_not_reached();
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the function to be called when program ends.  Note that this
  function should only be called once.
  \author  jfpatry
  \date    Created:  2000-10-20
  \date Modified: 2000-10-20 */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool_t called = False;

    check_assertion( called == False, "winsys_atexit called twice" );

    called = True;
    atexit_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Exits the program
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
void winsys_exit( int code )
{
    if ( atexit_func ) {
	(*atexit_func)();
    }
    
    exit( code );
}

#else

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* GLUT version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static winsys_keyboard_func_t keyboard_func = NULL;

static bool_t redisplay = False;


/*---------------------------------------------------------------------------*/
/*! 
  Requests that the screen be redrawn
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_post_redisplay() 
{
    redisplay = True;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the display callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_display_func( winsys_display_func_t func )
{
    glutDisplayFunc( func );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the idle callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_idle_func( winsys_idle_func_t func )
{
    glutIdleFunc( func );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the reshape callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    glutReshapeFunc( func );
}


/* Keyboard callbacks */
static void glut_keyboard_cb( unsigned char ch, int x, int y ) 
{
    if ( keyboard_func ) {
	(*keyboard_func)( ch, False, False, x, y );
    }
}

static void glut_special_cb( int key, int x, int y ) 
{
    if ( keyboard_func ) {
	(*keyboard_func)( key, True, False, x, y );
    }
}

static void glut_keyboard_up_cb( unsigned char ch, int x, int y ) 
{
    if ( keyboard_func ) {
	(*keyboard_func)( ch, False, True, x, y );
    }
}

static void glut_special_up_cb( int key, int x, int y ) 
{
    if ( keyboard_func ) {
	(*keyboard_func)( key, True, True, x, y );
    }
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the keyboard callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse button-press callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    glutMouseFunc( func );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse motion callback (when a mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_motion_func( winsys_motion_func_t func )
{
    glutMotionFunc( func );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse motion callback (when no mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    glutPassiveMotionFunc( func );
}



/*---------------------------------------------------------------------------*/
/*! 
  Copies the OpenGL back buffer to the front buffer
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_swap_buffers()
{
    glutSwapBuffers();
}


/*---------------------------------------------------------------------------*/
/*! 
  Moves the mouse pointer to (x,y)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_warp_pointer( int x, int y )
{
    glutWarpPointer( x, y );
}


/*---------------------------------------------------------------------------*/
/*! 
  Initializes the OpenGL rendering context, and creates a window (or 
  sets up fullscreen mode if selected)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_init( int *argc, char **argv, char *window_title, 
		  char *icon_title )
{
    int width, height;
    int glutWindow;

    glutInit( argc, argv );

#ifdef USE_STENCIL_BUFFER
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );
#else
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
#endif 

    /* Create a window */
    if ( getparam_fullscreen() ) {
	glutInitWindowPosition( 0, 0 );
	glutEnterGameMode();
    } else {
	/* Set the initial window size */
	width = getparam_x_resolution();
	height = getparam_y_resolution();
	glutInitWindowSize( width, height );

	if ( getparam_force_window_position() ) {
	    glutInitWindowPosition( 0, 0 );
	}

	glutWindow = glutCreateWindow( window_title );

	if ( glutWindow == 0 ) {
	    handle_error( 1, "Couldn't create a window." );
	}
    }
}


/*---------------------------------------------------------------------------*/
/*! 
  Deallocates resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_shutdown()
{
    if ( getparam_fullscreen() ) {
	glutLeaveGameMode();
    }
}

/*---------------------------------------------------------------------------*/
/*! 
  Enables/disables key repeat messages from being generated
  \return  
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_enable_key_repeat( bool_t enabled )
{
    glutIgnoreKeyRepeat(!enabled);
}

/*---------------------------------------------------------------------------*/
/*! 
  Shows/hides mouse cursor
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_show_cursor( bool_t visible )
{
    if ( visible ) {
	glutSetCursor( GLUT_CURSOR_LEFT_ARROW );
    } else {
	glutSetCursor( GLUT_CURSOR_NONE );
    }
}



/*---------------------------------------------------------------------------*/
/*! 
  Processes and dispatches events.  This function never returns.
  \return  No.
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_process_events()
{
    /* Set up keyboard callbacks */
    glutKeyboardFunc( glut_keyboard_cb );
    glutKeyboardUpFunc( glut_keyboard_up_cb );
    glutSpecialFunc( glut_special_cb );
    glutSpecialUpFunc( glut_special_up_cb );

    glutMainLoop();
}

/*---------------------------------------------------------------------------*/
/*! 
  Sets the function to be called when program ends.  Note that this
  function should only be called once.
  \author  jfpatry
  \date    Created:  2000-10-20
  \date Modified: 2000-10-20 */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool_t called = False;

    check_assertion( called == False, "winsys_atexit called twice" );

    called = True;

    atexit(func);
}


/*---------------------------------------------------------------------------*/
/*! 
  Exits the program
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
void winsys_exit( int code )
{
    exit(code);
}

#endif /* defined( HAVE_SDL ) */

/* EOF */
