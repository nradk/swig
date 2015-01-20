#include "Object.h"

Object::Object (unsigned vertex_count):
    m_vertex_count(vertex_count), m_vertex({4,vertex_count})
{
    // Initialize the points
    for(int i=0;i < vertexCount();i++)
        m_vertex(3,i) = 1;

    // Initialize edges and surfaces
}

// Load an object from an .obj file
Object::Object(const std::string& filename) : m_vertex({4,1}) {

    std::ifstream objfile(filename,std::ios::in);
    std::string line,keyword;
    Triplet<float> vtxpoint(0,0,0);
    unsigned facepoint;
    m_vertex_count = 0;

    if (objfile.fail()) {
        throw ex::InitFailure();
    }

    while (!objfile.eof()) {
        std::getline(objfile,line);
        if (line.size()<3)
            continue;
        std::istringstream linestrm(line);
        linestrm>>keyword;

        if (keyword=="v") {
            if (m_vertex_count!=0)
                m_vertex.addColumn();
            linestrm>>vtxpoint.x;
            linestrm>>vtxpoint.y;
            linestrm>>vtxpoint.z;
            m_vertex(0,m_vertex_count)=vtxpoint.x;
            m_vertex(1,m_vertex_count)=vtxpoint.y;
            m_vertex(2,m_vertex_count)=vtxpoint.z;
            m_vertex(3,m_vertex_count)=1;
            m_vertex_count++;
        } else if (keyword=="f") {
            std::vector<unsigned> face;
            while (!linestrm.eof()) {
                linestrm>>facepoint;
                face.push_back(facepoint-1);
            }
            std::vector<Triplet<unsigned> > tlst = tesselate(face);
            for (auto it=tlst.begin();it<tlst.end(); it++) {
                setSurface(*it);
            }
        }
    }
}

// Tesselate a polygon to triangles
std::vector<Triplet<unsigned> > Object::tesselate(
        std::vector<unsigned> face) {
    if (face.size()<3) {
        throw ex::InitFailure();
    } else {
        unsigned v1,v2,v3;
        std::vector<Triplet<unsigned> > tesselated;
        while (face.size()) {
            v1 = face.back(); face.pop_back();
            v2 = face.back(); face.pop_back();
            v3 = face.back(); face.pop_back();
            tesselated.push_back({v3,v2,v1});
            if (face.size()) {
                face.push_back(v3); face.push_back(v1);
            }
        }
        return tesselated;
    }
}

void Object::showVx() const {
    for (auto i=0; i<m_vertex_count; i++) {
        getVertex(i).display();
    }
}
