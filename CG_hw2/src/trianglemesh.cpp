#include "trianglemesh.h"

// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	// -------------------------------------------------------
	// Add your initialization code here.
	// -------------------------------------------------------
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	objExtent = glm::vec3(0.0f, 0.0f, 0.0f);
	vboId = 0;
}

// Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	// -------------------------------------------------------
	// Add your release code here.
	// -------------------------------------------------------
	vertices.clear();
	subMeshes.clear();
	ReleaseBuffers();
}

// Load the geometry and material data from an OBJ file.
bool TriangleMesh::LoadFromFile(const std::string& filePath, const bool normalized)
{	
	// Parse the OBJ file.
	// ---------------------------------------------------------------------------
    // Add your implementation here (HW1 + read *.MTL).
    // ---------------------------------------------------------------------------
	std::ifstream objfileIn(filePath);
	if (!objfileIn) {
		std::cerr << "Error: cannot open OBJ file: " << filePath << std::endl;
		return false;
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;

	std::string line;
	while (std::getline(objfileIn, line)) {
		std::istringstream iss(line);
		std::string type;
		iss >> type;
		if (type == "mtllib") {
			std::string mtlfileName;
			iss >> mtlfileName;
			std::string mtlfilePath = filePath;
			size_t lastSlashPos = mtlfilePath.find_last_of('/');
			if (lastSlashPos != std::string::npos) {
				mtlfilePath.replace(lastSlashPos + 1, std::string::npos, mtlfileName);
			}
			LoadMTLLib(mtlfilePath);
		}
		else if (type == "v") {
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
				subMeshes.back().vertexIndices.push_back(numVertices);
				subMeshes.back().vertexIndices.push_back(numVertices + i - 1);
				subMeshes.back().vertexIndices.push_back(numVertices + i);
			}
			// 更新頂點數量和三角形數量(頂點數 - 2)
			numVertices += cntVertices;
			numTriangles += cntVertices - 2;
		}
		else if (type == "usemtl") {
			std::string mtlName;
			iss >> mtlName;
			// 確保每個 material只會對應到一個 submesh
			auto it = std::find_if(subMeshes.begin(), subMeshes.end(), [&mtlName](const SubMesh& subMesh) {
				return subMesh.material && subMesh.material->GetName() == mtlName;
			});
			if (it == subMeshes.end()) {
				subMeshes.emplace_back();
				subMeshes.back().material = materials[mtlName];
			}
			else {
				std::iter_swap(it, std::prev(subMeshes.end()));
			}
		}
	}

	objfileIn.close();

	// Normalize the geometry data.
	if (normalized) {
		// -----------------------------------------------------------------------
		// Add your normalization code here (HW1).
		// -----------------------------------------------------------------------
		// Normalize the geometry data.	
		// Add your code here.
		// ... 
		// 使用 openGL 初始化座標極值
		glm::vec3 minPosBound = glm::vec3(std::numeric_limits<float>::max()); // 正無窮
		glm::vec3 maxPosBound = glm::vec3(std::numeric_limits<float>::lowest()); // 確保是負無窮
		// Bounding Box
		for (auto&& vertex : vertices) {
			minPosBound = glm::min(minPosBound, vertex.position);
			maxPosBound = glm::max(maxPosBound, vertex.position);
		}
		// Center
		objCenter = minPosBound + (maxPosBound - minPosBound) * 0.5f;
		// maximal extent axis
		float maxLen = std::max(std::max(maxPosBound.x - minPosBound.x, maxPosBound.y - minPosBound.y), maxPosBound.z - minPosBound.z);
		// maximal extent axis equal to 1
		for (auto&& vertex : vertices) {
			vertex.position = (vertex.position - objCenter) / maxLen;
		}
		// Extent
		objExtent = (maxPosBound - minPosBound) / maxLen;
	}

	return true;
}


bool TriangleMesh::LoadMTLLib(const std::string& filePath)
{
	std::ifstream mtlfileIn(filePath);
	if (!mtlfileIn) {
		std::cerr << "Error: cannot open MTL file: " << filePath << std::endl;
		return false;
	}

	std::string line;
	std::string currMtlName;
	while (std::getline(mtlfileIn, line)) {
		std::istringstream iss(line);
		std::string type;
		iss >> type;
		if (type == "newmtl") {
			iss >> currMtlName;
			materials[currMtlName] = new PhongMaterial();
			materials[currMtlName]->SetName(currMtlName);
		}
		else if (type == "Ns") {
			float n;
			iss >> n;
			materials[currMtlName]->SetNs(n);
		}
		else if (type == "Ka") {
			float r, g, b;
			iss >> r >> g >> b;
			materials[currMtlName]->SetKa(glm::vec3(r, g, b));
		}
		else if (type == "Kd") {
			float r, g, b;
			iss >> r >> g >> b;
			materials[currMtlName]->SetKd(glm::vec3(r, g, b));
		}
		else if (type == "Ks") {
			float r, g, b;
			iss >> r >> g >> b;
			materials[currMtlName]->SetKs(glm::vec3(r, g, b));
		}
	}

	mtlfileIn.close();

	return true;
}

// Desc: Create vertex buffer and index buffer.
void TriangleMesh::CreateBuffers()
{
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(VertexPTN), vertices.data(), GL_STATIC_DRAW);
	
	for (auto&& subMesh : subMeshes) {
		glGenBuffers(1, &(subMesh.iboId));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh.iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, subMesh.vertexIndices.size() * sizeof(unsigned int), subMesh.vertexIndices.data(), GL_STATIC_DRAW);
	}	
}

void TriangleMesh::ReleaseBuffers()
{
    glDeleteBuffers(1, &vboId);

	for (auto&& subMesh : subMeshes) {
		glDeleteBuffers(1, &(subMesh.iboId));
	}
}

void TriangleMesh::Render()
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (const GLvoid*)12);

	for (auto&& subMesh : subMeshes) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh.iboId);
		glDrawElements(GL_TRIANGLES, (GLsizei)(subMesh.vertexIndices.size()), GL_UNSIGNED_INT, 0);
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void TriangleMesh::RenderSubMesh(SubMesh subMesh)
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (const GLvoid*)12);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh.iboId);
	glDrawElements(GL_TRIANGLES, (GLsizei)(subMesh.vertexIndices.size()), GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// Show model information.
void TriangleMesh::ShowInfo()
{
	std::cout << "# Vertices: " << numVertices << std::endl;
	std::cout << "# Triangles: " << numTriangles << std::endl;
	std::cout << "Total " << subMeshes.size() << " subMeshes loaded" << std::endl;
	for (unsigned int i = 0; i < subMeshes.size(); ++i) {
		const SubMesh& g = subMeshes[i];
		std::cout << "SubMesh " << i << " with material: " << g.material->GetName() << std::endl;
		std::cout << "Num. triangles in the subMesh: " << g.vertexIndices.size() / 3 << std::endl;
	}
	std::cout << "Model Center: " << objCenter.x << ", " << objCenter.y << ", " << objCenter.z << std::endl;
	std::cout << "Model Extent: " << objExtent.x << " x " << objExtent.y << " x " << objExtent.z << std::endl;
}

void TriangleMesh::Debug()
{
	for (auto&& subMesh : subMeshes) {
		std::cout << subMesh.material->GetName() << std::endl;
		std::cout << "Ns: " << subMesh.material->GetNs() << std::endl;
		std::cout << "Ka: " << subMesh.material->GetKa().x << " " << subMesh.material->GetKa().y << " " << subMesh.material->GetKa().z << std::endl;
		std::cout << "Kd: " << subMesh.material->GetKd().x << " " << subMesh.material->GetKd().y << " " << subMesh.material->GetKd().z << std::endl;
		std::cout << "Ks: " << subMesh.material->GetKs().x << " " << subMesh.material->GetKs().y << " " << subMesh.material->GetKs().z << std::endl;
		/*
		for (int i = 0; i < subMesh.vertexIndices.size(); i++) {
			// std::cout << "Pos: " << vertices[subMesh.vertexIndices[i]].position.x << " " << vertices[subMesh.vertexIndices[i]].position.y << " " << vertices[subMesh.vertexIndices[i]].position.z << std::endl;
			std::cout << "Pos: " << subMesh.vertexIndices[i] << std::endl;
		}
		*/
	}
	
}
