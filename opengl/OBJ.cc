#include "opengl/OBJ.hh"

#include <fstream>
#include <cstdio>
#include <stdexcept>

#include "util/Log.hh"
#include "opengl/TextureManager.hh"

using namespace glm;

namespace opengl {

static bool startsWith(char const* start, std::string const& subject) {
    size_t i = 0;
    while (start[i]) {
        if (i >= subject.size())
            return false;

        if (start[i] != subject[i])
            return false;

        i++;
    }

    return true;
}

OBJ::Material::Material()
    : texture(nullptr) {
}

OBJ::Part::Part(Material material, Buffer<vec3>* vertices,
        Buffer<vec2>* texCoords, Buffer<vec3>* normals) 
    : material(material), vertices(vertices), texCoords(texCoords),
      normals(normals) {
}

OBJ::OBJ(std::string const& filename, TextureManager& textures) {
    load(filename, textures); 
}

OBJ::MaterialLib OBJ::loadMaterialLib(std::string const& filename,
                                      TextureManager& textures) {
    auto error = [&] (std::string const& error) {
        throw std::runtime_error("Error while loading MTL file \"" +
                                 filename + "\": " + error);
    };

    std::ifstream file(filename, std::ifstream::in);

    if (!file.good())
        error("Cannot open file \"" + filename + "\" for reading");

    MaterialLib mtllib;
    MaterialLib::iterator material = mtllib.end();

    std::string line;
    while (file.good()) {
        std::getline(file, line); 

        if (line.size() < 2)
            continue;
        
        // "Kd r g b": Color
        if (line[0] == 'K' && line[1] == 'd') {
            if (material == mtllib.end())
                error("No material name specified");

            if (sscanf(line.c_str(), "Kd %f %f %f",
                        &material->second.diffuse.x,
                        &material->second.diffuse.y,
                        &material->second.diffuse.z) != 3)
                error("Invalid Kd line");
        }
        
        // "map_Kd file": Texture
        else if (startsWith("map_Kd ", line)) {
            if (material == mtllib.end())
                error("No material name specified");
            if (material->second.texture)
                error("Multiple textures specified");

            material->second.texture = textures.load(line.substr(7));
        }
        
        // "newmtl name": Add new material
        else if (startsWith("newmtl ", line)) {
            auto ins = mtllib.insert(
                    std::make_pair(line.substr(7), Material()));
            if (!ins.second)
                error("Material specified twice");

            material = ins.first;
        }
    }

    return mtllib;
}

void OBJ::load(std::string const& filename, TextureManager& textures) {
    int lineNumber = 0;
    auto error = [&] (std::string const& error) {
            std::cout << lineNumber << std::endl;
        throw std::runtime_error("Error while loading OBJ file \"" +
                                 filename + "\": " + error);
    };

    std::ifstream file(filename, std::ifstream::in);

    if (!file.good())
        error("Cannot open file \"" + filename + "\" for reading");

    // Given by the .mtl file specified by "mtllib".
    MaterialLib mtllib;

    // The vertices, texCoords and normals specified by lines starting with
    // "v", "vt" and "vn" respectively are loaded into these vectors, so that
    // they may be referenced by "f" lines (faces).
    std::vector<vec3> vertices;
    std::vector<vec2> texCoords;
    std::vector<vec3> normals;

    // These vectors hold the current vertices, texCoords and normals as given
    // by the "f" lines. Everytime either a "usemtl" or a "o" (= object) line
    // is encountered, the vectors are emptied and a new Part is added to the
    // OBJ using their contents.
    std::vector<vec3> curVertices;
    std::vector<vec2> curTexCoords;
    std::vector<vec3> curNormals;

    // Last material specified by "usemtl".
    OBJ::Material curMaterial;

    // Add a new part using the information in cur*. Clears cur*.
    auto addPart = [&] () {
        if (curVertices.empty())
            return;
        if (!curTexCoords.empty() && !curNormals.empty()) {
            if (curVertices.size() != curNormals.size() ||
                curNormals.size() != curTexCoords.size())
                error("Inconsistent faces");

            parts.emplace_back(
                curMaterial,
                new Buffer<vec3>(GL_ARRAY_BUFFER, curVertices),
                new Buffer<vec2>(GL_ARRAY_BUFFER, curTexCoords),
                new Buffer<vec3>(GL_ARRAY_BUFFER, curNormals)
            );
        } else if (!curTexCoords.empty()) {
            if (curVertices.size() != curTexCoords.size())
                error("Inconsistent faces");  
            parts.emplace_back(
                curMaterial,
                new Buffer<vec3>(GL_ARRAY_BUFFER, curVertices),
                new Buffer<vec2>(GL_ARRAY_BUFFER, curTexCoords),
                nullptr
            );
        } else {
            if (curVertices.size() != curNormals.size())
                error("Inconsistent faces");

            parts.emplace_back( 
                curMaterial,
                new Buffer<vec3>(GL_ARRAY_BUFFER, curVertices),
                nullptr,
                new Buffer<vec3>(GL_ARRAY_BUFFER, curNormals)
            );
        }

        curVertices.clear();
        curTexCoords.clear();
        curNormals.clear();
    };

    std::string line;
    while (file.good()) {
        std::getline(file, line);
        lineNumber++;

        if (line.size() < 2)
            continue;

        // "v x y z": Add vertex
        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            if (sscanf(line.c_str(), "v %f %f %f", &x, &y, &z) != 3)
                error("Invalid vertex line");
            vertices.push_back(vec3(x, y, z));
        }
 
        // "vt x y z": Add texcoord
        else if (line[0] == 'v' && line[1] == 't') {
            float u, v;
            if (sscanf(line.c_str(), "vt %f %f", &u, &v) != 2)
                error("Invalid texcoord line");
            texCoords.push_back(vec2(u, v));
        }
        
        // "vn x y z": Add normal
        else if (line[0] == 'v' && line[1] == 'n') {
            float x, y, z;
            if (sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z) != 3)
                error("Invalid normal line");
            normals.push_back(vec3(x, y, z));
        }

        // "f (v/vt/vn)*": Add faces
        else if (line[0] == 'f') {
            // I feel the need to defend myself for writing this code

            size_t v1, vt1, vn1,
                   v2, vt2, vn2,
                   v3, vt3, vn3;
            if (sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                        &v1, &vt1, &vn1,
                        &v2, &vt2, &vn2,
                        &v3, &vt3, &vn3) == 9) {
                curVertices.push_back(vertices.at(v1 - 1)); 
                curVertices.push_back(vertices.at(v2 - 1)); 
                curVertices.push_back(vertices.at(v3 - 1)); 

                curTexCoords.push_back(texCoords.at(vt1 - 1));
                curTexCoords.push_back(texCoords.at(vt2 - 1));
                curTexCoords.push_back(texCoords.at(vt3 - 1));

                curNormals.push_back(normals.at(vn1 - 1));
                curNormals.push_back(normals.at(vn2 - 1));
                curNormals.push_back(normals.at(vn3 - 1));
            } else if (sscanf(line.c_str(), "f %d//%d %d//%d %d//%d",
                        &v1, &vn1, &v2, &vn2, &v3, &vn3) == 6) {
                // No texCoords given

                curVertices.push_back(vertices.at(v1 - 1)); 
                curVertices.push_back(vertices.at(v2 - 1)); 
                curVertices.push_back(vertices.at(v3 - 1)); 

                curNormals.push_back(normals.at(vn1 - 1));
                curNormals.push_back(normals.at(vn2 - 1));
                curNormals.push_back(normals.at(vn3 - 1));
            } else if (sscanf(line.c_str(), "f %d/%d %d/%d %d/%d",
                        &v1, &vt1, &v2, &vt2, &v3, &vt3)) {
                // No normals given

                curVertices.push_back(vertices.at(v1 - 1)); 
                curVertices.push_back(vertices.at(v2 - 1)); 
                curVertices.push_back(vertices.at(v3 - 1)); 

                curTexCoords.push_back(texCoords.at(vt1 - 1));
                curTexCoords.push_back(texCoords.at(vt2 - 1));
                curTexCoords.push_back(texCoords.at(vt3 - 1));
            } else {
                error("Unrecognized face format");
            }
        }

        // "usemtl name": Change material
        else if (startsWith("usemtl ", line)) {
            addPart();
            
            auto it = mtllib.find(line.substr(7));
            if (it == mtllib.end())
                error("Material not found");

            curMaterial = it->second;
        }

        // "o": New object
        else if (line[0] == 'o' || line[0] == 'g') {
            addPart();    
        }

        // "mtllib file.mtl": Load material lib
        else if (startsWith("mtllib ", line)) {
            if (!mtllib.empty())
                error("More than one mtllib specified");

            mtllib = loadMaterialLib(line.substr(7), textures);
        }
    }

    addPart();

    INFO(graphics) << "Loaded OBJ file \"" << filename << "\" with "
                   << parts.size() << " parts";
}

} // namespace game
