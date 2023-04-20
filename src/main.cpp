#include <iostream>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Image.h"
#include "triangle.h"
#include "boundingBox.h"
#include "drawing.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

double RANDOM_COLORS[7][3] = {
	{0.0000,    0.4470,    0.7410},
	{0.8500,    0.3250,    0.0980},
	{0.9290,    0.6940,    0.1250},
	{0.4940,    0.1840,    0.5560},
	{0.4660,    0.6740,    0.1880},
	{0.3010,    0.7450,    0.9330},
	{0.6350,    0.0780,    0.1840},
};

int main(int argc, char **argv)
{
	if(argc < 3) {
		cout << "Usage: ./A6 <SCENE> <IMAGE SIZE> <IMAGE FILENAME>" << endl;
		return 0;
	}
	int scene = atoi(argv[1]);
    int imgSize = atoi(argv[2]);
    string imgName(argv[3]);

    // Create the image. We're using a `shared_ptr`, a C++11 feature.
    auto image = make_shared<Image>(imgSize, imgSize);

	// Load geometry
	vector<float> posBuf; // list of vertex positions
	vector<float> norBuf; // list of vertex normals
	vector<float> texBuf; // list of vertex texture coords
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			size_t index_offset = 0;
			for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);
					if(!attrib.normals.empty()) {
						norBuf.push_back(attrib.normals[3*idx.normal_index+0]);
						norBuf.push_back(attrib.normals[3*idx.normal_index+1]);
						norBuf.push_back(attrib.normals[3*idx.normal_index+2]);
					}
					if(!attrib.texcoords.empty()) {
						texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+0]);
						texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+1]);
					}
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}
	cout << "Number of vertices: " << posBuf.size()/3 << endl;
    
    //organize posBuf
    vector<Vertex> vertices = toVertices(posBuf);
    
    //add normals to vertices and set normal as default rgb values for each vertex
    normals(vertices,norBuf);
    normalsColor(vertices,norBuf);
    
    //organize vectors into triangles
    vector<Triangle> triangles = toTriangles(vertices);
    
    //calculate bounds of whole object
    vector<float> bounds = findBounds(vertices);    //{xMin,xMax,yMin,yMax,zMin,zMax}
    
    //scale and translate
    scaleTranslate(imgSize, imgSize, bounds, triangles);
    
    switch (scene) {
        //Task 1: Drawing Bounding Boxes
        case 1:
        {
            int color[3];
            for(int i=0; i<triangles.size(); i++){
                color[0] = RANDOM_COLORS[i%7][0]*255;   //r
                color[1] = RANDOM_COLORS[i%7][1]*255;   //g
                color[2] = RANDOM_COLORS[i%7][2]*255;   //b
                drawBox(triangles[i], color, image);
            }
            
        }
            break;
        //Task 2: Drawing Triangles
        case 2:
        {
            int color[3];
            for(int i=0; i<triangles.size(); i++){
                color[0] = RANDOM_COLORS[i%7][0]*255;   //r
                color[1] = RANDOM_COLORS[i%7][1]*255;   //g
                color[2] = RANDOM_COLORS[i%7][2]*255;   //b
                drawTriangle(triangles[i], color, image);
            }
        }
            break;
        //Task 3: interpolating Per-Vertex Colors
        case 3:
        {
            int x=0;
            for(int i=0; i<triangles.size(); i++){
                //assign colors to vertices of triangle
                //a
                triangles[i].a.r = RANDOM_COLORS[x%7][0]*255;   //r
                triangles[i].a.g = RANDOM_COLORS[x%7][1]*255;   //g
                triangles[i].a.b = RANDOM_COLORS[x%7][2]*255;   //b
                x++;
                //b
                triangles[i].b.r = RANDOM_COLORS[x%7][0]*255;   //r
                triangles[i].b.g = RANDOM_COLORS[x%7][1]*255;   //g
                triangles[i].b.b = RANDOM_COLORS[x%7][2]*255;   //b
                x++;
                //c
                triangles[i].c.r = RANDOM_COLORS[x%7][0]*255;   //r
                triangles[i].c.g = RANDOM_COLORS[x%7][1]*255;   //g
                triangles[i].c.b = RANDOM_COLORS[x%7][2]*255;   //b
                x++;
                
                drawPerVertexTriangle(triangles[i], image);
            }
        }
            break;
        //Task 4: Vertical Color
        case 4:
        {
            for(int i=0; i<triangles.size(); i++){
                drawVerticalColor(triangles[i], bounds, image);
            }
        }
            break;
        //Task 5: Z-Buffering
        case 5:
        {
            vector<Vertex> zBuffer;
            for(int i=0; i<triangles.size(); i++){
                createZColoredZBuffer(triangles[i], bounds, zBuffer);
            }
            sort(zBuffer.begin(), zBuffer.end(), sortZBuffer);
            drawZBuffer(zBuffer, bounds, image);
        }
            break;
        //Task 6: Normal Coloring
        case 6:
        {
            //normal colors are already set by default
            vector<Vertex> zBuffer;
            for(int i=0; i<triangles.size(); i++){
                createZBuffer(triangles[i], bounds, zBuffer);
            }
            sort(zBuffer.begin(), zBuffer.end(), sortZBuffer);
            drawZBuffer(zBuffer, bounds, image);
        }
            break;
        //Task 7: Simple Lighting
        case 7:
        {
            
            vector<Vertex> zBuffer;
            for(int i=0; i<triangles.size(); i++){
                //calculate lighting
                double xC = 255 * triangles[i].a.n1 * (1/sqrt(3));
                double yC = 255 * triangles[i].a.n2 * (1/sqrt(3));
                double zC = 255 * triangles[i].a.n3 * (1/sqrt(3));
                unsigned char c = max(xC + yC + zC, 0.0);
                triangles[i].a.r = c; triangles[i].a.g = c; triangles[i].a.b = c;
                xC = 255 * triangles[i].b.n1 * (1/sqrt(3));
                yC = 255 * triangles[i].b.n2 * (1/sqrt(3));
                zC = 255 * triangles[i].b.n3 * (1/sqrt(3));
                c = max(xC + yC + zC, 0.0);
                triangles[i].b.r = c; triangles[i].b.g = c; triangles[i].b.b = c;
                xC = 255 * triangles[i].c.n1 * (1/sqrt(3));
                yC = 255 * triangles[i].c.n2 * (1/sqrt(3));
                zC = 255 * triangles[i].c.n3 * (1/sqrt(3));
                c = max(xC + yC + zC, 0.0);
                triangles[i].c.r = c; triangles[i].c.g = c; triangles[i].c.b = c;
                createZBuffer(triangles[i], bounds, zBuffer);
            }
            sort(zBuffer.begin(), zBuffer.end(), sortZBuffer);
            drawZBuffer(zBuffer, bounds, image);
        }
            break;
        default:
        {
            cout << "Task Not Found" << endl;
        }
            break;
    }
    image->writeToFile(imgName);
	return 0;
}
