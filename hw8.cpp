// HW8 for CS 637
// Shangqi Wu

#include "Angel.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

typedef vec4 color4;

// Height and width of main window. 
int h = 500;
int w = 500;

// True for perspective projection, false for parallel projection. 
bool perspective = true;
// If implementing phong shading. 
bool phong = true;
// There are 3 objects on screen, if left object is picked, it becomes 1, 2 for middle and 3 for right one.
int pick = 0;
// The flag for multisample anti-aliasing, set to true by default.
bool aa = true;
// A flag for displaying which color render buffer.
bool displayBuffer = true;

// RGBA color for background of main window.
float Red = 0;
float Green = 0;
float Blue = 0;
float Alpha = 1;

// Radius of camera rotation and its delta.
float cameraRadius = 2;
float dr = 0.05;
// Height of camera and its delta.
float cameraHeight = 0;
float dh = 0.05;
// Current position and its delta of a circling camera
float t = 0;
float dt = 0.01;

// Initial position of look-at, camera position, and up vector for projection.
vec4 at( 0, 0, 0, 1 );
vec4 eye( 0, 0, 0, 1 );
vec4 up( 0, 1, 0, 1 );

// Phong shading model parameters of light source.
const vec4 light1_pos( 0, 0, 4, 1 );
float Idr1 = 0.7;
float Idg1 = 0.7;
float Idb1 = 0.7;
float Isr1 = 0.2;
float Isg1 = 0.2;
float Isb1 = 0.2;
float Iar1 = 0.1;
float Iag1 = 0.1;
float Iab1 = 0.1;

// Shininess parameter for phong shading.
float shininess1 = 1;
float shininess2 = 100;
float shininess3 = 10000;

// Phong shading model parameters of material property. 
float k1dr = 0;
float k1dg = 1;
float k1db = 1;
float k1da = 0.5;
float k1sr = 0;
float k1sg = 1;
float k1sb = 1;
float k1sa = 0.5;
float k1ar = 0;
float k1ag = 1;
float k1ab = 1;
float k1aa = 0.5;
float k2dr = 1;
float k2dg = 0;
float k2db = 1;
float k2da = 0.5;
float k2sr = 1;
float k2sg = 0;
float k2sb = 1;
float k2sa = 0.5;
float k2ar = 1;
float k2ag = 0;
float k2ab = 1;
float k2aa = 0.5;
float k3dr = 1;
float k3dg = 1;
float k3db = 0;
float k3da = 0.5;
float k3sr = 1;
float k3sg = 1;
float k3sb = 0;
float k3sa = 0.5;
float k3ar = 1;
float k3ag = 1;
float k3ab = 0;
float k3aa = 0.5;

// IDs for main window. 
int MainWindow;

// Vector containing vertices of every triangle to be drawn.
vector<vec4> vertices;
// Vector for average normal for corresponding vertice in a triangle. 
vector<vec4> normals;
// Number of vertices to be drawn of each object.
int obj_size[3];
// Coordinate offsets for 3 objects. The program makes first one in the left, second one in the middle, while third one in the right.
float obj_offset[3] = { -1.5, 0, 1.5 };

// ID for VAO.
GLuint vao[1];
// ID for VBO.
GLuint buffer;
// ID for shader program.
GLuint program;
// ID for FBO.
GLuint frameBuffer;
// ID for color render buffer.
GLuint color_rb[2];
// ID for depth render buffer.
GLuint depth_rb;
// Indicator of multisample capacity of current platform.
GLint samples;

//--------------------------------------------------------------------------

vec4
product( const vec4 &a, const vec4 &b )
{
    return vec4( a[0]*b[0], a[1]*b[1], a[2]*b[2], a[3]*b[3] );
}

//--------------------------------------------------------------------------

void
init( void )
{
    // Create frame buffer object. 
    glGenFramebuffersEXT( 1, &frameBuffer );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, frameBuffer );

    // Collect data for multisamplin anti-aliasing.
    glGetIntegerv( GL_MAX_SAMPLES_EXT, &samples );

    if (!aa) samples = 0; // If anti-aliasing is shut down, set multisample buffer as normal buffer. 

    // Create 2 color buffers, 1 for display, the other for object identification.
    glGenRenderbuffersEXT( 2, color_rb );
    glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, color_rb[0] );
    glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER_EXT, samples, GL_RGBA8, w, h );
    glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, color_rb[0] );
    glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, color_rb[1] );
    glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER_EXT, samples, GL_RGBA8, w, h );
    glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_RENDERBUFFER_EXT, color_rb[1] );

    // Create depth buffer. 
    glGenRenderbuffersEXT( 1, &depth_rb );
    glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, depth_rb );
    glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER_EXT, samples, GL_DEPTH_COMPONENT24, w, h );
    glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_rb );

    // Check buffer status. 
    GLenum status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        cerr<<"Frame buffer object has error."<<endl;
        exit(1);
    }

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

    // Create a vertex array object.
    glGenVertexArrays( 1, vao );
    glBindVertexArray( vao[0] );
    cout<<"glGenVertexArrays(), glBindVertexArray() for main window initialization."<<endl;

    // Create and initialize a buffer object.
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    cout<<"glGenBuffer(), glBindBuffer() for main window initialization."<<endl;
    glBufferData( GL_ARRAY_BUFFER, vertices.size()*sizeof(vec4)+normals.size()*sizeof(vec4), NULL, GL_STATIC_DRAW );
    // Pass vertices & normals data to opengl buffer object.
    glBufferSubData( GL_ARRAY_BUFFER, 0, vertices.size()*sizeof(vec4), vertices.data() );
    glBufferSubData( GL_ARRAY_BUFFER, vertices.size()*sizeof(vec4), normals.size()*sizeof(vec4), normals.data() );
    cout<<"glBufferData(), glBufferSubData() for main window initialization."<<endl;

    // Load shaders and use the resulting shader program.
    program = InitShader( "vshader21.glsl", "fshader21.glsl" );
    LinkShader( program );
    glUseProgram( program );
    cout<<"InitShader(), glUseProgram() for main window initialization."<<endl;

    // Initialize the vertex position attribute from the vertex shader.
    GLuint loc_ver = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( loc_ver );
    glVertexAttribPointer( loc_ver, 4, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(0) );

    // Pass normal vectors of each triangle to vertex shader
    GLuint loc_col = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( loc_col );
    glVertexAttribPointer( loc_col, 4, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET( vertices.size()*sizeof(vec4) ) );
    cout<<"glEnableVertexAttribArray(), glVertexAttribPointer() for main window initialization."<<endl;
}

//----------------------------------------------------------------------------

void
recalMatrix( void )
{
    // Calculate renewed camera position. 
    eye = vec4( cameraRadius*sin(t), cameraHeight, cameraRadius*cos(t), 1 );

    // Create model and projection matrix.
    mat4 modelview = Angel::identity();
    mat4 projection = Angel::identity();

    // Implementing projection.
    if (perspective) projection *= Perspective( 90, 1, 0.000001, 1 );
    else projection *= Ortho( -3, 3, -3, 3, -100, 100 );

    // Implementing modelview. 
    modelview *= LookAt( eye, at, up );
    
    // Pass model and projection matrix to vertex shader. 
    GLuint loc_modelview = glGetUniformLocation( program, "modelview" );
    glUniformMatrix4fv( loc_modelview, 1, GL_TRUE, modelview );
    GLuint loc_projection = glGetUniformLocation( program, "projection" );
    glUniformMatrix4fv( loc_projection, 1, GL_TRUE, projection );
    GLuint loc_eyeposition = glGetUniformLocation( program, "eyeposition" );
    glUniform4f( loc_eyeposition, eye.x, eye.y, eye.z, eye.w );

    // Pass positions of light sources to vertex shader.
    GLuint loc_light1_pos = glGetUniformLocation( program, "light1_pos" );
    glUniform4f( loc_light1_pos, light1_pos.x, light1_pos.y, light1_pos.z, light1_pos.w );

    cout<<"glGetUniformLocation(), glUniformMatrix4fv() for transformation matrix."<<endl;
}

//----------------------------------------------------------------------------

void
display( void )
{
    recalMatrix(); // Calculates vertices & colors for objects in main window. 

    // Preparation for off-screen rendering.
    GLenum drawbuffers[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, frameBuffer );
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, frameBuffer );
    glClearColor( Red, Green, Blue, Alpha );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, w, h );

    // Set light property.
    color4 light1_diffuse = color4( Idr1, Idg1, Idb1, 1 );
    color4 light1_specular = color4( Isr1, Isg1, Isb1, 1 );
    color4 light1_ambient = color4( Iar1, Iag1, Iab1, 1 );
    color4 material_diffuse, material_ambient, material_specular;
    // Material property for first object. If it is picked, the object becomes red.
    if (pick == 1) {
        material_diffuse = vec4( 1, 0, 0, 1 );
        material_ambient = vec4( 1, 0, 0, 1 );
        material_specular = vec4( 1, 0, 0, 1 );
    } else {
        material_diffuse = vec4( k1dr, k1dg, k1db, k1da );
        material_ambient = vec4( k1ar, k1ag, k1ab, k1aa );
        material_specular = vec4( k1sr, k1sg, k1sb, k1sa );
    }
    // Calculate and pass color products of each light source to vertex shader.
    vec4 d_pro = product( light1_diffuse, material_diffuse );
    vec4 a_pro = product( light1_ambient, material_ambient );
    vec4 s_pro = product( light1_specular, material_specular );
    GLint loc_diffuse_product = glGetUniformLocation( program, "light1_diffuse_product" );
    GLint loc_specular_product = glGetUniformLocation( program, "light1_specular_product" );
    GLint loc_ambient_product = glGetUniformLocation( program, "light1_ambient_product" );
    glUniform4f( loc_diffuse_product, d_pro.x, d_pro.y, d_pro.z, d_pro.w );
    glUniform4f( loc_specular_product, s_pro.x, s_pro.y, s_pro.z, s_pro.w );
    glUniform4f( loc_ambient_product, a_pro.x, a_pro.y, a_pro.z, a_pro.w );
    GLint loc_shininess = glGetUniformLocation( program, "shininess" );
    glUniform1f( loc_shininess, shininess1 );

    // Draw first object. 
    glDrawBuffers( 2, drawbuffers );
    glDrawArrays( GL_TRIANGLES, 0, obj_size[0] ); 

    // Modify material property for the second object, red if picked.
    if (pick == 2) {
        material_diffuse = vec4( 1, 0, 0, 1 );
        material_ambient = vec4( 1, 0, 0, 1 );
        material_specular = vec4( 1, 0, 0, 1 );
    } else {
        material_diffuse = vec4( k2dr, k2dg, k2db, k2da );
        material_ambient = vec4( k2ar, k2ag, k2ab, k2aa );
        material_specular = vec4( k2sr, k2sg, k2sb, k2sa );
    }
    // Calculate and pass color products of each light source to vertex shader.
    d_pro = product( light1_diffuse, material_diffuse );
    a_pro = product( light1_ambient, material_ambient );
    s_pro = product( light1_specular, material_specular );
    glUniform4f( loc_diffuse_product, d_pro.x, d_pro.y, d_pro.z, d_pro.w );
    glUniform4f( loc_specular_product, s_pro.x, s_pro.y, s_pro.z, s_pro.w );
    glUniform4f( loc_ambient_product, a_pro.x, a_pro.y, a_pro.z, a_pro.w );
    loc_shininess = glGetUniformLocation( program, "shininess" );
    glUniform1f( loc_shininess, shininess2 );

    // Draw the second object.
    glDrawBuffers( 2, drawbuffers );
    glDrawArrays( GL_TRIANGLES, obj_size[0], obj_size[1] );
    
    // Modify material property for the third object, red if picked.
    if (pick == 3) {
        material_diffuse = vec4( 1, 0, 0, 1 );
        material_ambient = vec4( 1, 0, 0, 1 );
        material_specular = vec4( 1, 0, 0, 1 );
    } else {
        material_diffuse = vec4( k3dr, k3dg, k3db, k3da );
        material_ambient = vec4( k3ar, k3ag, k3ab, k3aa );
        material_specular = vec4( k3sr, k3sg, k3sb, k3sa );
    }
    // Calculate and pass color products of each light source to vertex shader.
    d_pro = product( light1_diffuse, material_diffuse );
    a_pro = product( light1_ambient, material_ambient );
    s_pro = product( light1_specular, material_specular );
    glUniform4f( loc_diffuse_product, d_pro.x, d_pro.y, d_pro.z, d_pro.w );
    glUniform4f( loc_specular_product, s_pro.x, s_pro.y, s_pro.z, s_pro.w );
    glUniform4f( loc_ambient_product, a_pro.x, a_pro.y, a_pro.z, a_pro.w );
    loc_shininess = glGetUniformLocation( program, "shininess" );
    glUniform1f( loc_shininess, shininess3 );

    // Draw the last object.
    glDrawBuffers( 2, drawbuffers );
    glDrawArrays( GL_TRIANGLES, obj_size[0]+obj_size[1], obj_size[2] );

    // Set frame buffer and copy rendered image into screen. 
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
    glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, frameBuffer );
    if (displayBuffer) glReadBuffer( GL_COLOR_ATTACHMENT0_EXT );
    else glReadBuffer( GL_COLOR_ATTACHMENT1_EXT );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glBlitFramebuffer( 0, 0, w-1, h-1, 0, 0, w-1, h-1, GL_COLOR_BUFFER_BIT, GL_NEAREST );

    glutSwapBuffers(); // Double buffer swapping. 
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ); // Unbind frame buffer. 
    glFlush(); // Flush. 
    cout<<"glClearColor(), glClear(), glDrawArrays(), glutSwapBuffers(), glFlush() for main window display function."<<endl;
}

//----------------------------------------------------------------------------

void
RotationFunc( void )
{
    t += dt; // Camera rotation animation.
    glutPostRedisplay(); // Redisplay function.
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
        case 033: exit( EXIT_SUCCESS ); break; // "Esc": exit the program.
        case (int)'w': cameraHeight += dh; break; // Increasing camera height.
        case (int)'s': cameraHeight -= dh; break; // Decreasing camera height.
        case (int)'a': cameraRadius += dr; break;// Incresing camera radius, the object looks smaller under perspective projection.
        case (int)'d': if (cameraRadius > dr) cameraRadius -= dr; break; // Decreasing camera radius, the object looks larger under perspective projection.
        case (int)'e': t += dt; break; // Double camera rotation speed.
        case (int)'q': t -= dt; break; // Half camera rotation speed.
        case (int)'r': t = 0; cameraHeight = 0; cameraRadius = 2; glutIdleFunc( NULL ); break;
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void 
MainSubMenuRotation( int id )
{
    switch ( id ) {
        case 1: glutIdleFunc( RotationFunc ); break; // Start or stop camera rotation.
        case 2: glutIdleFunc( NULL ); break; // Start or stop light rotation.
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void 
MainSubMenuPerspective( int id )
{
    switch ( id ) {
        case 1: perspective = true; break; // Switch to persepctive projection.
        case 2: perspective = false; break; // Switch to parallel projection.
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
MainSubMenuAA( int id )
{
    switch ( id ) { // The menu for turning on or down AA.
        case 1: 
            if (!aa) { // If AA is already on, such tedious work should be skipped. 
                aa = true; // Trun on the flag, and reinitialize buffers with multisampling.
                glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
                glDeleteRenderbuffersEXT( 2, color_rb );
                glDeleteRenderbuffersEXT( 1, &depth_rb );
                glDeleteFramebuffersEXT( 1, &frameBuffer );
                glDeleteBuffers( 1, vao );
                glDeleteBuffers( 1, &buffer );
                init();
            }
            break;
        case 2: 
           if (aa) { // If AA is already off, such tedious work should be skipped.
               aa = false; // Turn down the flag, and reinitialize buffers without multisampling.
               glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
               glDeleteRenderbuffersEXT( 2, color_rb );
               glDeleteRenderbuffersEXT( 1, &depth_rb );
               glDeleteFramebuffersEXT( 1, &frameBuffer );
               glDeleteBuffers( 1, vao );
               glDeleteBuffers( 1, &buffer );
               init();
           }
           break;
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
MainSubMenuBuffer( int id )
{
    switch ( id ) { // Changing buffer display mode.
        case 1: displayBuffer = true; break;
        case 2: displayBuffer = false; break;
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
setMainWinMenu( void )
{
    int submenu_id_r, submenu_id_p, submenu_id_a, submenu_id_b;
    // Create submenu for rotating animation.
    submenu_id_r = glutCreateMenu( MainSubMenuRotation );
    glutAddMenuEntry( "Start Camera Rotation", 1 );
    glutAddMenuEntry( "Stop Camera Rotation", 2 );

    // Create submenu for projection changing.
    submenu_id_p = glutCreateMenu( MainSubMenuPerspective );
    glutAddMenuEntry( "Perspective Projection", 1 );
    glutAddMenuEntry( "Parallel Projection", 2 );

    // Create submenu for selecting anti-aliasing mode. 
    submenu_id_a = glutCreateMenu( MainSubMenuAA );
    glutAddMenuEntry( "Enable Anti-Aliasing", 1 );
    glutAddMenuEntry( "Disable Anti-Aliasing", 2 );

    // Create subment for buffer selection.
    submenu_id_b = glutCreateMenu( MainSubMenuBuffer );
    glutAddMenuEntry( "Color Buffer for Screen", 1 );
    glutAddMenuEntry( "Color Buffer for Objects Identification", 2 );

    glutCreateMenu( NULL ); // Set menu in main window. 
    cout<<"glutCreateMenu() for main window menu."<<endl;
    glutAddSubMenu( "Anti-Aliasing", submenu_id_a );
    glutAddSubMenu( "Color Buffer Selection", submenu_id_b );
    glutAddSubMenu( "Camera Rotation", submenu_id_r );
    glutAddSubMenu( "Projection", submenu_id_p );
    cout<<"glutAddMenuEntry() for main window menu."<<endl;
    glutAttachMenu( GLUT_RIGHT_BUTTON );
    cout<<"glutAttachMenu() for main window menu."<<endl;
}

//----------------------------------------------------------------------------

void
MouseClick( GLint button, GLint state, GLint x, GLint y )
{
    // This function can pick up the object clicked by the mouse.
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Y coordinate correction.
        y = h - y;
        GLuint readFBO, readColor, readDepth;

        if (aa && samples > 1) {
            // Since multisample FBO is imcompatible with pixel reading, a normal FBO is created. 
            glGenFramebuffersEXT( 1, &readFBO );
            glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, readFBO );

            glGenRenderbuffersEXT( 1, &readColor );
            glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, readColor );
            glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_RGBA8, w, h );
            glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, readColor );

            glGenRenderbuffersEXT( 1, &readDepth );
            glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, readDepth );
            glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h );
            glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, readDepth );
            // Copy data from multisample FBO to normal FBO, then set normal FBO as read buffer.
            glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, frameBuffer );
            glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, readFBO );
            glReadBuffer( GL_COLOR_ATTACHMENT1_EXT );
            glBlitFramebufferEXT( 0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST );
            glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
            
            glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, readFBO );
        } else { // No AA performed, just read and sample as usual.
            glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, frameBuffer );
            glReadBuffer( GL_COLOR_ATTACHMENT1_EXT );
        }

        // Create variable to get the data of specific pixel. 
        GLubyte * myimage = new GLubyte[4];
        glReadPixels( x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, myimage );

        if (myimage[0] == 0 && myimage[1] == 0 && myimage[2] == 0) {
            pick = 0; // If black background, cancel any picked object.
        } else if (myimage[0] == 0 && myimage[1] != 0 && myimage[2] != 0) {
            pick = 1; // If cyan one, it indicates the first object is selected.
        } else if (myimage[0] != 0 && myimage[1] == 0 && myimage[2] != 0) {
            pick = 2; // If magenta one, it indicates the second object is selected.
        } else if (myimage[0] != 0 && myimage[1] != 0 && myimage[2] == 0) {
            pick = 3; // If yellow one, it indicates the third object is selected.
        }

        // Unbind, delete all unnecessary buffers and data.
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
        if (aa && samples > 1) {
            glDeleteRenderbuffersEXT( 1, &readColor );
            glDeleteRenderbuffersEXT( 1, &readDepth );
            glDeleteFramebuffersEXT( 1, &readFBO );
        }
        delete [] myimage;
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
ChangeSize( int new_w, int new_h )
{
    // The function to keep window size up to date.
    w = new_w;
    h = new_h;
    // After the window is reshaped, buffers should be reallocated for the new size.
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
    glDeleteRenderbuffersEXT( 2, color_rb );
    glDeleteRenderbuffersEXT( 1, &depth_rb );
    glDeleteFramebuffersEXT( 1, &frameBuffer );
    glDeleteBuffers( 1, vao );
    glDeleteBuffers( 1, &buffer );
    init();
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void 
readfile( void )
{
    // Start to read file.
    ifstream infile;
    int files = 0;
    cout<<"Please enter 3 files separately."<<endl;

    do {
        vector<vec4> original_vertices;
        vector<vec4> original_normals;

        do {
            string input;
            cout<<"Please enter one input smf file name here:"<<endl;
            cin>>input;
            infile.open(input.c_str());
            if (!infile) cout<<"Your input file path is incorrect."<<endl;
        } while (!infile);

        bool storev = false;
        bool storef = false;
        string str;
        vector<vector<int> > faces;
        vector<double> ver_pos;
        vector<int> ver_no;

        // Read file content.
        while (infile) {
            infile>>str;
            if (str.compare("v") == 0) {
                storev = true;
            } else if (str.compare("f") == 0){
                storef = true;
            } else if (storev == true){ // Add vertex to its vector.
                // Store a vertice.
                ver_pos.push_back(atof(str.c_str()));
                if (ver_pos.size() == 3) {
                    vec4 ver( ver_pos[0]+obj_offset[files], ver_pos[1], ver_pos[2], 1 );
                    original_vertices.push_back(ver);
                    storev = false;
                    ver_pos.clear();
                }
            } else if (storef == true){ // Add face to its vector.
                ver_no.push_back(atoi(str.c_str()));
                // Store vertices for a triangle and calculate its normal vector.
                if (ver_no.size() == 3) {
                    faces.push_back(ver_no);
                    storef = false;
                    ver_no.clear();
                }
            }
        }
        infile.close();

        for (int i = 0; i < (int)original_vertices.size(); i++)
            original_normals.push_back( vec4( 0, 0, 0, 0 ) );

        for (int i = 0; i < (int)faces.size(); i++) {
            vec4 tmpnorm = normalize( vec4( cross( (original_vertices[faces[i][1]-1]-original_vertices[faces[i][0]-1]), (original_vertices[faces[i][2]-1]-original_vertices[faces[i][0]-1]) ) ) );
            original_normals[faces[i][0]-1] = normalize( original_normals[faces[i][0]-1] + tmpnorm );
            original_normals[faces[i][1]-1] = normalize( original_normals[faces[i][1]-1] + tmpnorm );
            original_normals[faces[i][2]-1] = normalize( original_normals[faces[i][2]-1] + tmpnorm );
        }

        for (int i = 0; i < (int)faces.size(); i++) {
            vertices.push_back( original_vertices[faces[i][0]-1] );
            vertices.push_back( original_vertices[faces[i][1]-1] );
            vertices.push_back( original_vertices[faces[i][2]-1] );
            normals.push_back( original_normals[faces[i][0]-1] );
            normals.push_back( original_normals[faces[i][1]-1] );
            normals.push_back( original_normals[faces[i][2]-1] );
        }

        obj_size[files] = static_cast<int>(vertices.size());
        
    } while (++files < 3);

    obj_size[2] -= obj_size[1];
    obj_size[1] -= obj_size[0];
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{   
    readfile(); // Read input smf file.

    glutInit( &argc, argv ); // Initializing environment.
    cout<<"glutInit(&argc,argv) called."<<endl;
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE ); // Enable depth.
    cout<<"glutInitDisplayMode() called."<<endl;
    glutInitWindowSize( w, h );
    cout<<"glutInitWindowSize() called."<<endl;
    glutInitWindowPosition( 100, 100 );
    cout<<"glutInitWindowPosition() called."<<endl;

    MainWindow = glutCreateWindow( "ICG_hw8" ); // Initializing & setting main window.
    cout<<"glutCreateWindow() for main window."<<endl;
    glewExperimental=GL_TRUE; 
    glewInit();    
    cout<<"glewInit() for main window."<<endl;

    glEnable( GL_BLEND ); 
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    //glEnable( GL_POINT_SMOOTH );
    //glEnable( GL_LINE_SMOOTH );
    //glEnable( GL_POLYGON_SMOOTH );
    cout<<"GL_BLEND enabled."<<endl;

    init(); // Initializing VAOs & VBOs. 
    glutDisplayFunc( display ); // Setting display function for main window.
    cout<<"glutDisplayFunc() for main window."<<endl;
    glutKeyboardFunc( keyboard ); // Setting keyboard function for main window.
    cout<<"glutKeyboardFunc() for main window."<<endl;
    glutMouseFunc( MouseClick );
    setMainWinMenu(); // Setting menu for main window. 
    glutIdleFunc( NULL ); // Start animation by default.
    cout<<"glutIdleFunc() for main window."<<endl;
    glutReshapeFunc( ChangeSize );

    glEnable( GL_MULTISAMPLE_ARB );
    glEnable( GL_DEPTH_TEST );
    cout<<"glutMainLoop() called."<<endl;
    
    glutMainLoop(); // Start main loop. 
    return 0;
}
