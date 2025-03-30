#include "TriangleMesh.h"


// Desc: Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	vboId = 0;
	iboId = 0;
}

// Desc: Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	vertices.clear();
	vertexIndices.clear();
	glDeleteBuffers(1, &vboId);
	glDeleteBuffers(1, &iboId);
}

// Desc: Load the geometry data of the model from file and normalize it.
bool TriangleMesh::LoadFromFile(const std::string& filePath, const bool normalized)
{	
	// Add your code here.
	// ... 
	std::ifstream fileIn(filePath);
	if (!fileIn) {
		std::cerr << "Error: cannot open OBJ file: " << filePath << std::endl;
		return false;
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;

	std::string line = "";
	while (std::getline(fileIn, line)) {
		// 區分資料
		std::istringstream iss(line);
		std::string type;
		iss >> type;
		if (type == "v") {
			float x, y, z;
			iss >> x >> y >> z;
			positions.emplace_back(x, y, z);
		}
		else if (type == "vn") {
			float x, y, z;
			iss >> x >> y >> z;
			normals.emplace_back(x, y, z);
		}
		else if (type == "vt") {
			float u, v;
			iss >> u >> v;
			texcoords.emplace_back(u, v);
		}
		else if (type == "f") {
			int cntVertices = 0;
			std::string facedata;
			while (iss >> facedata) {
				std::istringstream vertiss(facedata);
				std::string posIndexStr, texcoordIndexStr, normalIndexStr;		// f P/T/N
				std::getline(vertiss, posIndexStr, '/');
				std::getline(vertiss, texcoordIndexStr, '/');
				std::getline(vertiss, normalIndexStr, '/');
				// 資料是 base 1
				int posIndex = std::stoi(posIndexStr) - 1;
				int texcoordIndex = std::stoi(texcoordIndexStr) - 1;
				int normalIndex = std::stoi(normalIndexStr) - 1;
				vertices.emplace_back(positions[posIndex], normals[normalIndex], texcoords[texcoordIndex]);
				cntVertices++;
			}

			// 多邊形分割
			// HW1_slides V1~V7 : 七個點五個三角形 i.e., n 個點會有 n - 2 個三角形
			// V1-V2-V3: 0-1-2
			// V1-V3-V4: 0-2-3
			// V1-V4-V5: 0-3-4
			// V1-V5-V6: 0-4-5
			// V1-V6-V7: 0-5-6
			for (int i = 2; i < cntVertices; i++) {		// 共用第一個點，故從2開始
				vertexIndices.push_back(numVertices);
				vertexIndices.push_back(numVertices + i - 1);
				vertexIndices.push_back(numVertices + i);
			}
			// 更新頂點數量和三角形數量(頂點數 - 2)
			numVertices += cntVertices;
			numTriangles += cntVertices - 2;
		}
	}

	fileIn.close();

	// Find the minimal bounding box of the 3D model
	// Find the center of the 3D model
	// Find the maximal extent axis of the bounding box
	// Find a mapping to make the model located at the origin and its maximal extent axis equal to 1

	if (normalized) {
		// Normalize the geometry data.	
		// Add your code here.
		// ... 
		// 使用 openGL 初始化座標極值
		glm::vec3 minPosBound = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 maxPosBound = glm::vec3(std::numeric_limits<float>::min());
		// Bounding Box
		for (auto&& vertex : vertices) {
			minPosBound = glm::min(minPosBound, vertex.position);
			maxPosBound = glm::max(maxPosBound, vertex.position);
		}
		// Center
		objCenter = minPosBound + (maxPosBound - minPosBound) * 0.5f;
		// maximal extent axis
		float maxLen = std::max(std::max(maxPosBound.x - minPosBound.x, maxPosBound.y - minPosBound.y), maxPosBound.z - minPosBound.z);
		//maximal extent axis equal to 1
		for (auto&& vertex : vertices) {
			vertex.position = (vertex.position - objCenter) / maxLen;
		}
	}

	PrintMeshInfo();
	return true;
}

// Desc: Create vertex buffer and index buffer.
void TriangleMesh::CreateBuffers()
{
	// Add your code here.
	// ...
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(VertexPTN), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &iboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(unsigned int),vertexIndices.data(), GL_STATIC_DRAW);
}

// Desc: Apply transformation to all vertices (DON'T NEED TO TOUCH)
void TriangleMesh::ApplyTransformCPU(const glm::mat4x4& mvpMatrix)
{
	for (int i = 0 ; i < numVertices; ++i) {
        glm::vec4 p = mvpMatrix * glm::vec4(vertices[i].position, 1.0f);
        if (p.w != 0.0f) {
            float inv = 1.0f / p.w; 
            vertices[i].position.x = p.x * inv;
            vertices[i].position.y = p.y * inv;
            vertices[i].position.z = p.z * inv;
        }
    }
}

// Desc: Print mesh information.
void TriangleMesh::PrintMeshInfo() const
{
	std::cout << "[*] Mesh Information: " << std::endl;
	std::cout << "# Vertices: " << numVertices << std::endl;
	std::cout << "# Triangles: " << numTriangles << std::endl;
	std::cout << "Center: (" << objCenter.x << " , " << objCenter.y << " , " << objCenter.z << ")" << std::endl;
}
