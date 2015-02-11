#include <iostream>
#include <cstring>

#include "mathematics/Vector.h"
#include "mathematics/Matrix.h"
#include "TfMatrix.h"

#include "Drawer.h"

#include "PointLight.h"
#include "AmbientLight.h"
#include "Object.h"

#include "misc/Time.h"

// Initialize constant parameters
const uint16_t WIDTH = 800;
const uint16_t HEIGHT = 600;
const uintmax_t FPS = 100;
const uintmax_t DELAY = 1e6/FPS;

int main(int argc, char* argv[]) {

    if (argc<2) {
        std::cout<<"Usage: "<<argv[0]<<" filename"<<std::endl;
        return 1;
    }

    Color black = {0,0,0,255};
    Color white = {255,255,255,255};
    Color badcolor = {255,0,255,255};

    // Gourad Shading
    bool GOURAD = false;
    // Enable two face
    bool TWOFACE = false;
    // Bacface detection
    bool BACKFACEDETECTION = true;
    if(TWOFACE)
        BACKFACEDETECTION = false;

    // Initialize the plotter interface
    Plotter_ fb(WIDTH,HEIGHT);
    Drawer drawer(&fb);
    drawer.clear();

    // Intialize the benchmark
    Time timekeeper;

    // Intialize the point light sources
    std::vector<PointLight> light;
    light.push_back({{-100,100,100,0},  {20000,0,0}});
    light.push_back({{0,100,100,0},     {0,15000,0}});
    light.push_back({{0,0,100,0},       {0,0,20000}});
    light.push_back({{0,-100,100,0},    {10000,10000,10000}});

    // Intialize the ambient light sources
    AmbientLight ambient = {{100,100,100}};

    // Initialize the object
    Object obj(argv[1]);
    obj.material.ka = {0.1,0.1,0.1};
    obj.material.kd = {0.5,0.5,0.5};
    obj.material.ks = {0.5,0.5,0.5};
    obj.material.ns = 140;
    //obj.vmatrix() /= TfMatrix::translation({0,0,-2});

    unsigned nSurfs = obj.surfaceCount();
    unsigned nVerts = obj.vertexCount();

    std::cout << "FPS limit:\t" << FPS << "\n" << std::endl;

    while (!fb.checkTerm()) {

        // Start benchmark time
        timekeeper.start();

        // Initialize camera
        // View reference point
        // View plane normal
        // View up vector
        Vector vrp(0,0,6);
        Vector vpn = Vector(0,0,0) - vrp;
        Vector vup(0,1,-1);

        // Vertex Shader
        // Intialize a transformation matrix to transform object
        // Apply transformation to object
        // Apply transformation to vertex normals (only rotation type)
        Matrix<float> rotator = TfMatrix::rotation(Math::toRadian(2),Vector(1,1,0),Vector(0,0,0));
        obj.vmatrix() /= rotator;
        obj.nmatrix() /= rotator;

        // Surface Shader
        // Detect backfaces in normalized co-ordinates
        bool* show = new bool[nSurfs];
        if(BACKFACEDETECTION){
            memset(show,1,nSurfs*sizeof(bool));
            // Required for backface detection
            // When backface is off, still can be useful for TWOFACE objects
            // to reverse the direction

            for(int i=0;i<nSurfs;i++) {
            Vector normal = obj.getSurfaceNormal(obj.getSurface(i));
            Vector position = obj.getSurfaceCentroid(i);
            if( Vector::cosine((position-vrp),normal) > 0)
                show[i] = false;
            // TODO validate the previous method, may be buggy in
            // perspective projection
            /*
                Vector normal = copy.getSurfaceNormal(i);
                if( normal.z >= 0)
                    show[i] = false;
                    */
            }
        }

        // Vertex / Surface Shader
        // Gourad or Flat shading
        unsigned iteration = GOURAD?nVerts:nSurfs;
        // Stores Color for each vertices
        Color* colors = new Color[iteration];

        // Calcualte Shading
        for(auto i=0;i<iteration;i++) {

            Vector normal = GOURAD?
                obj.getVertexNormal(i):
                obj.getSurfaceNormal(obj.getSurface(i));

            Vector position = GOURAD?
                obj.getVertex(i):
                obj.getSurfaceCentroid(i);


            // TODO some bad pixels seen on the boundaries
            // Still better than previous alternative
            // Good for flat shading than gourad
            // May improve when using vertex normals from loaded file
            //   if(show[i]==false)
            //   normal = -normal;
            if( TWOFACE && Vector::cosine((position-vrp),normal) > 0)
                normal = normal * -1;

            // Ambient lighting
            Coeffecient intensity = ambient.intensity * obj.material.ka;
            for(int i=0; i<light.size(); i++){
                Coeffecient decintensity= light[i].intensityAt(position);
                Vector decdirection = light[i].directionAt(position);
                // Diffused lighting
                {
                    float cosine = Vector::cosine((decdirection*(-1)),normal);
                    if(cosine <= 0)
                        continue;
                    intensity += (decintensity * obj.material.kd) * cosine;
                }
                // Specular ligting
                {
                    Vector half = (decdirection + (position-vrp).normalized())*(-1);
                    float cosine = Vector::cosine(half,normal);
                    if(cosine <= 0)
                        continue;
                    intensity += (decintensity * obj.material.ks) * std::pow(cosine, obj.material.ns);
                }
            }
            // The reflection surface can be seen as a light source to camera
            colors[i] = PointLight({position,intensity}).intensityAt(vrp);
        }


        // Vertex Shader
        // Get the vector copy matrix which is
        // initialized to the itself
        obj.resetCopy();
        Matrix<float>& copyalias = obj.vcmatrix();
        // Apply perspective projection and camera projection on copy
        copyalias /=
            TfMatrix::perspective2(95,(float)WIDTH/HEIGHT,10000,5)
            * TfMatrix::lookAt(vrp,vpn,vup);
        // Change the homogenous co-ordinates to
        // normalized co-ordinate and  to device co-ordinate
        for (unsigned i=0; i<nVerts; i++) {
            // Perspective divide
            // NOTE: normalization is also done during the projection transformation
            copyalias(0,i) /= copyalias(3,i);
            copyalias(1,i) /= copyalias(3,i);
            copyalias(2,i) /= copyalias(3,i);

            // Converting normalized Z co-ordinate [-1,1] to depthmap [1,0]*depth
            // Visible part is between n and f.
            // The z map is shown below
            // space:        0       n       f
            // value: -ve   inf      1       0   -ve
            // NOTE: n and f have negative values
            // Normalized co-rodinate is in right hand system
            // camera is towards negative Z axis
            copyalias(2,i) = (-copyalias(2,i)*0.5 + 0.5)*ScreenPoint::maxDepth;
            // Change the normalized X Y co-ordinates to device co-ordinate
            copyalias(0,i) = copyalias(0,i)*WIDTH + WIDTH/2;
            copyalias(1,i) = HEIGHT - (copyalias(1,i)*HEIGHT + HEIGHT/2);
        }


        // Clear framebuffer, we're about to plot
        drawer.clear(white);

        // Fill the surfaces
        for (int i=0; i<nSurfs; i++) {

            if(BACKFACEDETECTION && !show[i])
                continue;

            int index = obj.getSurface(i).x;
            ScreenPoint a(obj.getCopyVertex(index),colors[GOURAD?index:i]);

            index = obj.getSurface(i).y;
            ScreenPoint b(obj.getCopyVertex(index),colors[GOURAD?index:i]);

            index = obj.getSurface(i).z;
            ScreenPoint c(obj.getCopyVertex(index),colors[GOURAD?index:i]);

            if (GOURAD)
                drawer.fillD(a,b,c);
            else
                drawer.fillD(a,b,c,colors[i]);
        }

        // Update framebuffer
        drawer.update();

        delete []show;
        delete []colors;

        // Stop benchmark time
        timekeeper.stop();
        uintmax_t ti = timekeeper.time();
        float real_fps = 1e6 / ti;
        // Convert us to ms and delay
        if( ti < DELAY){
            SDL_Delay((DELAY-ti)/1000);
        }

        std::cout << DELETE;
        std::cout << "Nolimit FPS:\t" << real_fps << std::endl;
    }
    return 0;
}
