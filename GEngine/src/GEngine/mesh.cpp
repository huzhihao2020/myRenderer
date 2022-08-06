#include "mesh.h"

GEngine::CMesh::CMesh(std::vector<SVertex> vertices,
                      std::vector<unsigned int> indices,
                      std::vector<STexture> textures) {
  this->vertices_ = vertices;
  this->indices_ = indices;
  this->textures_ = textures;
  // now that we have all the required data, set the vertex buffers and its
  // attribute pointers.
  SetupMesh();
}

GEngine::CMesh::~CMesh() {}

// render the mesh
void GEngine::CMesh::Draw(std::shared_ptr<CShader> shader) {
  unsigned int diffuseNr = 1;
  unsigned int specularNr = 1;
  unsigned int normalNr = 1;
  unsigned int heightNr = 1;
  for (unsigned int i = 0; i < textures_.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    std::string number_str;
    std::string name = textures_[i].type_;
    if (name == "texture_diffuse")
      number_str = std::to_string(diffuseNr++);
    else if (name == "texture_specular")
      number_str =
          std::to_string(specularNr++); // transfer unsigned int to string
    else if (name == "texture_normal")
      number_str =
          std::to_string(normalNr++); // transfer unsigned int to string
    else if (name == "texture_height")
      number_str =
          std::to_string(heightNr++); // transfer unsigned int to string

    glUniform1i(
        glGetUniformLocation(shader->GetShaderID(), (name + number_str).c_str()),
        i);
    glBindTexture(GL_TEXTURE_2D, textures_[i].id_);
  }

  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices_.size()),
                 GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  // always good practice to set everything back to defaults once configured.
  glActiveTexture(GL_TEXTURE0);
}

void GEngine::CMesh::SetupMesh() {
  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // A great thing about structs is that their memory layout is sequential for
  // all its items. The effect is that we can simply pass a pointer to the
  // struct and it translates perfectly to a glm::vec3/2 array which again
  // translates to 3/2 floats which translates to a byte array.
  glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(SVertex),
               &vertices_[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int),
               &indices_[0], GL_STATIC_DRAW);

  // set the vertex attribute pointers
  // vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), (void *)0);
  // vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex),
                        (void *)offsetof(SVertex, normal_));
  // vertex texture coords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SVertex),
                        (void *)offsetof(SVertex, tex_coords_));
  // vertex tangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex),
                        (void *)offsetof(SVertex, tangent_));
  // vertex bitangent
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex),
                        (void *)offsetof(SVertex, bitangent_));
  // ids
  glEnableVertexAttribArray(5);
  glVertexAttribIPointer(5, 4, GL_INT, sizeof(SVertex),
                         (void *)offsetof(SVertex, bone_id_));

  // weights
  glEnableVertexAttribArray(6);
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(SVertex),
                        (void *)offsetof(SVertex, weights_));
  glBindVertexArray(0);
}
