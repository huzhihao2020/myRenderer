#include "model.h"

#include "stb/stb_image.h"

// constructor, expects a filepath to a 3D model.
GEngine::CModel::CModel(std::string const &path, bool gamma) {
  gamma_correction_ = gamma;
  LoadModel(path);
}

GEngine::CModel::~CModel() {}

// draws the model, and thus all its meshes
void GEngine::CModel::Draw(CShader &shader) {
  for (unsigned int i = 0; i < meshes_.size(); i++)
    meshes_[i].Draw(shader);
}

// loads a model with supported ASSIMP extensions from file and stores the
// resulting meshes in the meshes vector.
void GEngine::CModel::LoadModel(std::string const &path) {
  // read file via ASSIMP
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  // check for errors
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) // if is Not Zero
  {
    std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
    return;
  }
  // retrieve the directory path of the filepath
  directory_ = path.substr(0, path.find_last_of('/'));

  // process ASSIMP's root node recursively
  ProcessNode(scene->mRootNode, scene);
}

// processes a node in a recursive fashion. Processes each individual mesh
// located at the node and repeats this process on its children nodes (if any).
void GEngine::CModel::ProcessNode(aiNode *node, const aiScene *scene) {
  // process each mesh located at the current node
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    // the node object only contains indices to index the actual objects in the
    // scene. the scene contains all the data, node is just to keep stuff
    // organized (like relations between nodes).
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes_.push_back(ProcessMesh(mesh, scene));
  }
  // after we've processed all of the meshes (if any) we then recursively
  // process each of the children nodes
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    ProcessNode(node->mChildren[i], scene);
  }
}

GEngine::CMesh GEngine::CModel::ProcessMesh(aiMesh *mesh,
                                            const aiScene *scene) {
  // data to fill
  std::vector<SVertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<STexture> textures;

  // walk through each of the mesh's vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    SVertex vertex;
    glm::vec3 vector; // we declare a placeholder vector since assimp uses
                      // its own vector class that doesn't directly convert
                      // to glm's vec3 class so we transfer the data to this
                      // placeholder glm::vec3 first.
    // positions
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.position_ = vector;
    // normals
    if (mesh->HasNormals()) {
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.normal_ = vector;
    }
    // texture coordinates
    if (mesh->mTextureCoords[0]) // does the mesh contain texture
                                 // coordinates?
    {
      glm::vec2 vec;
      // a vertex can contain up to 8 different texture coordinates. We thus
      // make the assumption that we won't use models where a vertex can
      // have multiple texture coordinates so we always take the first set
      // (0).
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.tex_coords_ = vec;
      // tangent
      vector.x = mesh->mTangents[i].x;
      vector.y = mesh->mTangents[i].y;
      vector.z = mesh->mTangents[i].z;
      vertex.tangent_ = vector;
      // bitangent
      vector.x = mesh->mBitangents[i].x;
      vector.y = mesh->mBitangents[i].y;
      vector.z = mesh->mBitangents[i].z;
      vertex.bitangent_ = vector;
    } else
      vertex.tex_coords_ = glm::vec2(0.0f, 0.0f);

    vertices.push_back(vertex);
  }
  // now wak through each of the mesh's faces (a face is a mesh its
  // triangle) and retrieve the corresponding vertex indices.
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    // retrieve all indices of the face and store them in the indices vector
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }
  // process materials
  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
  // we assume a convention for sampler names in the shaders. Each diffuse
  // texture should be named as 'texture_diffuseN' where N is a sequential
  // number ranging from 1 to MAX_SAMPLER_NUMBER. Same applies to other
  // texture as the following list summarizes: diffuse: texture_diffuseN
  // specular: texture_specularN
  // normal: texture_normalN

  // 1. diffuse maps
  std::vector<STexture> diffuseMaps =
      LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
  // 2. specular maps
  std::vector<STexture> specularMaps = LoadMaterialTextures(
      material, aiTextureType_SPECULAR, "texture_specular");
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  // 3. normal maps
  std::vector<STexture> normalMaps =
      LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
  // 4. height maps
  std::vector<STexture> heightMaps =
      LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
  textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

  // return a mesh object created from the extracted mesh data
  return CMesh(vertices, indices, textures);
}

// checks all material textures of a given type and loads the textures if
// they're not loaded yet. the required info is returned as a Texture struct.
std::vector<GEngine::STexture>
GEngine::CModel::LoadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                      std::string type_name) {
  std::vector<STexture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    // check if texture was loaded before and if so, continue to next iteration:
    // skip loading a new texture
    bool skip = false;
    for (unsigned int j = 0; j < textures_loaded_.size(); j++) {
      if (std::strcmp(textures_loaded_[j].path_.data(), str.C_Str()) == 0) {
        textures.push_back(textures_loaded_[j]);
        skip = true; // a texture with the same filepath has already been
                     // loaded, continue to next one. (optimization)
        break;
      }
    }
    if (!skip) { // if texture hasn't been loaded already, load it
      STexture texture;
      texture.id_ = GEngine::TextureFromFile(str.C_Str(), this->directory_);
      texture.type_ = type_name;
      texture.path_ = str.C_Str();
      textures.push_back(texture);
      textures_loaded_.push_back(
          texture); // store it as texture loaded for entire model, to ensure we
                    // won't unnecesery load duplicate textures.
    }
  }
  return textures;
}

unsigned int GEngine::TextureFromFile(const char *path,
                                      const std::string &directory,
                                      bool gamma) {
  std::string filename = std::string(path);
  filename = directory + '/' + filename;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data =
      stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}
