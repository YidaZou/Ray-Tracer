//
//  drawing.h
//  A1
//
//  Created by Yida Zou on 1/26/23.
//

#ifndef drawing_h
#define drawing_h

//draw bounding boxes
void drawBox(Triangle& t, int (&color)[3], std::shared_ptr<Image>& image){
    //bounds of triangle
    int xMin = std::min({t.a.x,t.b.x,t.c.x});
    int xMax = std::max({t.a.x,t.b.x,t.c.x});
    int yMin = std::min({t.a.y,t.b.y,t.c.y});
    int yMax = std::max({t.a.y,t.b.y,t.c.y});
    
    //drawing
    for(int y = yMin; y <= yMax; ++y) {
        for(int x = xMin; x <= xMax; ++x) {
            image->setPixel(x, y, color[0], color[1], color[2]);
        }
    }
}

//draw triangles
void drawTriangle(Triangle& t, int (&color)[3], std::shared_ptr<Image>& image){
    //bounds of triangle
    int xMin = std::min({t.a.x,t.b.x,t.c.x});
    int xMax = std::max({t.a.x,t.b.x,t.c.x});
    int yMin = std::min({t.a.y,t.b.y,t.c.y});
    int yMax = std::max({t.a.y,t.b.y,t.c.y});
    
    //drawing
    Vertex v;
    for(int y = yMin; y <= yMax; ++y) {
        for(int x = xMin; x <= xMax; ++x) {
            v.x = x; v.y = y;
            //std::cout<<"p: " <<v.x << " " << v.y << std::endl;
            if(isInside(t.a, t.b, t.c, v)){
                image->setPixel(x, y, color[0], color[1], color[2]);
            }
        }
    }
}

//draw per-vertex colored triangles
void drawPerVertexTriangle(Triangle& t, std::shared_ptr<Image>& image){
    //bounds of triangle
    int xMin = std::min({t.a.x,t.b.x,t.c.x});
    int xMax = std::max({t.a.x,t.b.x,t.c.x});
    int yMin = std::min({t.a.y,t.b.y,t.c.y});
    int yMax = std::max({t.a.y,t.b.y,t.c.y});
    
    //drawing
    Vertex v;
    for(int y = yMin; y <= yMax; ++y) {
        for(int x = xMin; x <= xMax; ++x) {
            v.x = x; v.y = y;
            if(isInside(t.a, t.b, t.c, v)){
                vector<unsigned char> color = colorWeight(t.a, t.b, t.c, v);    //color interpolation
                image->setPixel(x, y, color[0], color[1], color[2]);
            }
        }
    }
}

//draw vertical color triangles
void drawVerticalColor(Triangle& t, vector<float>& bounds, std::shared_ptr<Image>& image){
    //bounds of triangle
    int xMin = std::min({t.a.x,t.b.x,t.c.x});
    int xMax = std::max({t.a.x,t.b.x,t.c.x});
    int yMin = std::min({t.a.y,t.b.y,t.c.y});
    int yMax = std::max({t.a.y,t.b.y,t.c.y});
    
    //drawing
    Vertex v;
    for(int y = yMin; y <= yMax; ++y) {
        for(int x = xMin; x <= xMax; ++x) {
            v.x = x; v.y = y;
            if(isInside(t.a, t.b, t.c, v)){
                float yRelative = y - bounds[2];        //y-yMin
                float objHeight = bounds[3] - bounds[2];  //yMax-yMin
                float colorRatio = (objHeight - yRelative) / objHeight;
                image->setPixel(x, y, 255*(1-colorRatio), 0, 255*colorRatio);
            }
        }
    }
}

//create points to add to zBuffer and color accordingly
void createZColoredZBuffer(Triangle& t, vector<float>& bounds, vector<Vertex>& zBuffer){
    //bounds of triangle
    int xMin = std::min({t.a.x,t.b.x,t.c.x});
    int xMax = std::max({t.a.x,t.b.x,t.c.x});
    int yMin = std::min({t.a.y,t.b.y,t.c.y});
    int yMax = std::max({t.a.y,t.b.y,t.c.y});
    float zHeight = bounds[5] - bounds[4];  //zMax-zMin
    
    //drawing
    Vertex v;
    for(int y = yMin; y <= yMax; ++y) {
        for(int x = xMin; x <= xMax; ++x) {
            v.x = x; v.y = y;
            if(isInside(t.a, t.b, t.c, v)){
                //z still uses unscaled and untransformed values
                v.z = zWeight(t.a, t.b, t.c, v); //interpolate z value
                float zRelative = v.z - bounds[4]; //z-zMin
                float zRatio = zRelative / zHeight;
                
                v.r = 255*zRatio;
                v.g = 0; v.b = 0;
                zBuffer.push_back(v);
            }
        }
    }
}

//DONT ACTUALLY NEED THIS
//sort z buffer from farthest to closest
bool sortZBuffer(const Vertex& a, const Vertex& b){
    return a.z < b.z;
}

//draw from sorted zBuffer
void drawZBuffer(vector<Vertex>& zBuffer, vector<float>& bounds, std::shared_ptr<Image>& image){
    for(auto p : zBuffer){
        image->setPixel(p.x, p.y, p.r, p.g, p.b);
    }
}

//create points to add to zBuffer
void createZBuffer(Triangle& t, vector<float>& bounds, vector<Vertex>& zBuffer){
    //bounds of triangle
    int xMin = std::min({t.a.x,t.b.x,t.c.x});
    int xMax = std::max({t.a.x,t.b.x,t.c.x});
    int yMin = std::min({t.a.y,t.b.y,t.c.y});
    int yMax = std::max({t.a.y,t.b.y,t.c.y});
    
    //drawing
    Vertex v;
    for(int y = yMin; y <= yMax; ++y) {
        for(int x = xMin; x <= xMax; ++x) {
            v.x = x; v.y = y;
            if(isInside(t.a, t.b, t.c, v)){
                vector<unsigned char> color = colorWeight(t.a, t.b, t.c, v);    //color interpolation
                v.r = color[0]; v.g = color[1]; v.b = color[2];
                v.z = zWeight(t.a, t.b, t.c, v); //interpolate z value
                zBuffer.push_back(v);
            }
        }
    }
}

//create points to add to zBuffer
void createLightedZBuffer(Triangle& t, vector<float>& bounds, vector<Vertex>& zBuffer, unsigned char c){
    //bounds of triangle
    int xMin = std::min({t.a.x,t.b.x,t.c.x});
    int xMax = std::max({t.a.x,t.b.x,t.c.x});
    int yMin = std::min({t.a.y,t.b.y,t.c.y});
    int yMax = std::max({t.a.y,t.b.y,t.c.y});
    
    //drawing
    Vertex v;
    for(int y = yMin; y <= yMax; ++y) {
        for(int x = xMin; x <= xMax; ++x) {
            v.x = x; v.y = y;
            if(isInside(t.a, t.b, t.c, v)){
                v.r = c; v.g = c; v.b = c;
                v.z = zWeight(t.a, t.b, t.c, v); //interpolate z value
                zBuffer.push_back(v);
            }
        }
    }
}

#endif /* drawing_h */
