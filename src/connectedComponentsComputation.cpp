#include "connectedComponentsComputation.h"

#include <unordered_map>
#include <queue>
#include <algorithm> // array fill

#include <iostream>
#include "debug.h"

// namespace CuraConnectedComponents
//{



template<class Prim>
void fill_array(Prim* array, unsigned int size, Prim val)
{
    for (unsigned int i = 0; i < size; i++)
        array[i] = val;
}
    

void ConnectedComponentsComputation::buildMap()
{
    for (unsigned int face_idx = 0; face_idx < n_faces; face_idx++)
    {
        FaceRaw face(faces_raw, face_idx);
        vertex2faces.emplace(VertexRaw(vertices_raw, face.vertex_idx(0)), face);
        vertex2faces.emplace(VertexRaw(vertices_raw, face.vertex_idx(1)), face);
        vertex2faces.emplace(VertexRaw(vertices_raw, face.vertex_idx(2)), face);
    }
}

void ConnectedComponentsComputation::findNextUnassignedFace()
{
    while (face_part_idx[lastUnassignedFace] != -1 && lastUnassignedFace < n_faces)
    {
//             DEBUG_SHOW(*(face_part_idx + lastUnassignedFace));
        lastUnassignedFace++;
//             DEBUG_SHOW(*(face_part_idx + lastUnassignedFace));
    }
}

void ConnectedComponentsComputation::getComponent(unsigned int face_idx)
{
    typedef std::unordered_multimap<VertexRaw, FaceRaw, VertexRaw_Hasher>::iterator Vertex2face_it;
    std::queue<FaceRaw> todo; // contains the faces already added to the current connected component, but of which the neighbors are not yet processed
    todo.emplace(faces_raw, face_idx);
    face_part_idx[face_idx] = lastComponentId;
    while (!todo.empty())
    {
        FaceRaw& face_now = todo.front();
        
        for (int vert = 0; vert < 3; vert++)
        {
            unsigned int vert_idx = face_now.vertex_idx(vert);
            vert_part_idx[vert_idx] = lastComponentId;
            VertexRaw vertexRaw(vertices_raw, vert_idx);
            std::pair<Vertex2face_it, Vertex2face_it> connected_faces_range = vertex2faces.equal_range(vertexRaw);
            if(connected_faces_range.first == connected_faces_range.second)
            {
                DEBUG_PRINTLN("ERROR: disconnected vertex?!?!?!?! : " << vertexRaw.getCoord(0) << ", " << vertexRaw.getCoord(1) << ", " << vertexRaw.getCoord(2));
            }
            for (Vertex2face_it vertex_n_face = connected_faces_range.first; vertex_n_face != connected_faces_range.second; vertex_n_face++)
            {
                if (vertex_n_face->second == face_now) { continue; }
                FaceRaw& face_connected = vertex_n_face->second;
                if (face_part_idx[face_connected.idx] == -1) // if it wasnt in the component already
                {
                    todo.push(face_connected);
                }
                face_part_idx[face_connected.idx] = lastComponentId;
                
            }
        }
        
        
        todo.pop();
    }
    
    lastComponentId++;
}

    
ConnectedComponentsComputation::ConnectedComponentsComputation()
: lastUnassignedFace(0)
, lastComponentId(0)
{ 
}

// std::vector<Component> 
int ConnectedComponentsComputation::getComponents(float* vertices_raw, unsigned int n_verts, unsigned int* faces_raw, unsigned int n_faces)
{
    this->vertices_raw = vertices_raw;
    this->n_verts = n_verts;
    this->faces_raw = faces_raw;
    this->n_faces = n_faces;
    
    std::vector<Component> result;
    
    vert_part_idx = new int[n_verts];
    fill_array<int>(vert_part_idx, n_verts, -1);
    
    face_part_idx = new int[n_faces];
    fill_array<int>(face_part_idx, n_faces, -1);
    
    buildMap();
    
    findNextUnassignedFace();
    
    
    while (lastUnassignedFace < n_faces)
    {
        getComponent(lastUnassignedFace);
        findNextUnassignedFace();
    }
    unsigned int n_components = lastComponentId;
    
    unsigned int new_vert_idx[n_verts];
    fill_array<unsigned int>(new_vert_idx, n_verts, (unsigned int)0);
    
    {
        unsigned int comp_vert_count_current[n_components];
        fill_array<unsigned int>(comp_vert_count_current, n_components, (unsigned int)0);
        for (unsigned int v = 0; v < n_verts; v++)
        {
            new_vert_idx[v] = comp_vert_count_current[vert_part_idx[v]];
            ++ comp_vert_count_current[vert_part_idx[v]];
        }
    }
    
    for (unsigned int comp_id = 0; comp_id < n_components; comp_id++)
    {
        result.emplace_back();
    }
    
    for (unsigned int v = 0; v < n_verts; v++)
    {
        result[vert_part_idx[v]].verts.emplace_back(vertices_raw + v*3);
    }
    
    for (unsigned int f = 0; f < n_faces; f++)
    {
        FaceRaw face(faces_raw, f);
        result[face_part_idx[f]].faces.emplace_back(
                    new_vert_idx[face.vertex_idx(0)]
                , new_vert_idx[face.vertex_idx(1)]
                , new_vert_idx[face.vertex_idx(2)]
            );
    }
    
    for (Component& comp : result)
    {
        DEBUG_PRINTLN("verts:");
        for (Vertex& v : comp.verts)
            DEBUG_PRINTLN(v.x << ", " << v.y << ", " << v.z);
        DEBUG_PRINTLN("faces:");
        for (Face& f : comp.faces)
            DEBUG_PRINTLN(f.p0 <<", "<<f.p1<<", "<<f.p2);
    }

    delete[] vert_part_idx; 
    delete[] face_part_idx; 

//     return result;
    return 1;
}






//} // CuraConnectedComponents
