/*
    Copyright (C) 2010 Justin Crause <juzz@crause.com>

	UCT Honours Project 2010 - Deformable Terrain
*/

#include "JuzzUtility.h"

//--------------------------------------------------------------------------------
//	Class:	VBOData
//			Used to calculate and store the data for cube/plane creation
//--------------------------------------------------------------------------------

//Constructor
VBOData::VBOData(ShaderManager *n_shaderManager, float n_divs, float n_width, bool cube)
{
	//Store the shader manager
	shaderManager = n_shaderManager;
	shader = 0;

	//Set the number of divisions and calculate how many verticies
	divs = n_divs;
	numOfVerticies = (int)pow((divs + 1.0f), 2.0f);
	numOfIndicies = (int)pow(divs, 2.0f) * 6;

	//Set the width of the surfaces
	width = n_width;

	//There are 6 sides to the cube
	if (cube)
	{
		numOfVerticies *= 6;
		numOfIndicies *= 6;
	}

	//Set initial offsets
	curVertOffset = 0;
	curIndOffset = 0;

	//Initialize the variables
	verticies = new vector3[numOfVerticies];
	texcoords = new vector2[numOfVerticies];
	normals = new vector3[numOfVerticies];
	tangents = new vector3[numOfVerticies];
	normalCount = new int[numOfVerticies];
	indicies = new GLuint[numOfIndicies];

	printf("Number of verticies : %d\n", numOfVerticies);
	printf("Number of indicies  : %d\n", numOfIndicies);

	//Setup initial texture coordinate boundries
	textl = vector2(0.0f, 1.0f); 
	textr = vector2(1.0f, 1.0f);
	texbl = vector2(0.0f, 0.0f);

	//Either calculate a cube or plane
	if (cube)
		CalculateCube();
	else
		CalculatePlane();

	//Since multiple normals & tangents are calculated, calculate the average
	AverageNormalsTangents();

	//Bind the data to the VAO & VBO's
	BindVBOData();
}

//Destructor
VBOData::~VBOData(void)
{
	DeleteData();
	glDeleteBuffers(VBOCOUNT, VBO);
	glDeleteVertexArrays(1, &VAO);
}

//Draw the object
void VBOData::DrawObject(void)
{
	//Set the active shader
	shaderManager->SetActiveShader(shader);

	//Bind the VAO and draw
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numOfIndicies, GL_UNSIGNED_INT, 0);
}

//Change the shader this object uses
void VBOData::SetShader(int n_shader)
{
	shader = n_shader;
}

//Calculate the data for a entire cube
void VBOData::CalculateCube(void)
{
	//The eight verticies for a cube
	vector3 verts[8];
	//Front
	//Top Left
	verts[0][0] = -width;
	verts[0][1] = width;
	verts[0][2] = width;
	//Top Right
	verts[1][0] = width;
	verts[1][1] = width;
	verts[1][2] = width;
	//Bottom Right
	verts[2][0] = width;
	verts[2][1] = -width;
	verts[2][2] = width;
	//Bottom Left
	verts[3][0] = -width;
	verts[3][1] = -width;
	verts[3][2] = width;
	//Back
	//Top Left
	verts[4][0] = -width;
	verts[4][1] = width;
	verts[4][2] = -width;
	//Top Right
	verts[5][0] = width;
	verts[5][1] = width;
	verts[5][2] = -width;
	//Bottom Right
	verts[6][0] = width;
	verts[6][1] = -width;
	verts[6][2] = -width;
	//Bottom Left
	verts[7][0] = -width;
	verts[7][1] = -width;
	verts[7][2] = -width;

	//Calculate the data for the faces
	CalculateFace(verts[0], verts[1], verts[3]);
	CalculateFace(verts[1], verts[5], verts[2]);
	CalculateFace(verts[5], verts[4], verts[6]);
	CalculateFace(verts[4], verts[0], verts[7]);
	CalculateFace(verts[4], verts[5], verts[0]);
	CalculateFace(verts[3], verts[2], verts[7]);
}

//Calculate the data for a plane
void VBOData::CalculatePlane(void)
{
	//The eight verticies for a cube
	vector3 verts[3];
	//Front
	//Top Left
	verts[0][0] = -width;
	verts[0][1] = 0.0f;
	verts[0][2] = -width;
	//Top Right
	verts[1][0] = width;
	verts[1][1] = 0.0f;
	verts[1][2] = -width;
	//Bottom Left
	verts[2][0] = -width;
	verts[2][1] = 0.0f;
	verts[2][2] = width;

	verts[0][0] = -width;
	verts[0][1] = width;
	verts[0][2] = 0.0f;
	verts[1][0] = width;
	verts[1][1] = width;
	verts[1][2] = 0.0f;
	verts[2][0] = -width;
	verts[2][1] = -width;
	verts[2][2] = 0.0f;

	//Calculate data for a single face
	CalculateFace(verts[0], verts[1], verts[2]);
}

//Calculate the data for a specific face
void VBOData::CalculateFace(vector3 tl, vector3 tr, vector3 bl)
{
	//Calculate the increment vectors
	vector3 vInci = (bl - tl) / divs;
	vector3 vIncj = (tr - tl) / divs;
	vector2 texInci = (texbl - textl) / divs;
	vector2 texIncj = (textr - textl) / divs;

	//Loop over the surface
	//Need (divs + 1) since 1 div will need 2 verticies in each direction
	for (int i = 0; i < (divs + 1); i++)
	{
		for (int j = 0; j < (divs + 1); j++)
		{
			//Set the vertex position
			verticies[curVertOffset] = tl + (vInci * (float)i) + (vIncj * (float)j);

			//Set the texture coordinates
			texcoords[curVertOffset] = textl + (texInci * (float)i) + (texIncj * (float)j);

			//Initialize normal and tangent
			normals[curVertOffset] = vector3(0.0f, 0.0f, 0.0f);
			tangents[curVertOffset] = vector3(0.0f, 0.0f, 0.0f);
			normalCount[curVertOffset] = 0;

			//Do this if not the first column or row.
			//This calculates the normals, etc...
			if (i > 0 && j > 0)
			{
				//Create variables for position of verticies in quad
				int botR = curVertOffset;
				int botL = botR - 1;
				int topR = curVertOffset - ((int)divs + 1);
				int topL = topR - 1;

				//Calculate the index values
				indicies[curIndOffset++] = botR;
				indicies[curIndOffset++] = topR;
				indicies[curIndOffset++] = topL;
				indicies[curIndOffset++] = topL;
				indicies[curIndOffset++] = botL;
				indicies[curIndOffset++] = botR;

				//Calculate normals and tangents for:
				//Top-Left vertex
				CalculateNormalTangent(topL, topR, botL);
				//Top-Right vertex
				CalculateNormalTangent(topR, botR, topL);
				//Bottom-Right vertex
				CalculateNormalTangent(botR, botL, topR);
				//Bottom-Left vertex
				CalculateNormalTangent(botL, topL, botR);
			}

			//Increment the vertex offset value
			curVertOffset++;
		}
	}
}

//Calculate the normal and tangent and add to existing one, to be divided out later
void VBOData::CalculateNormalTangent(int vertCenter, int vertLeft, int vertRight)
{
	//Calculate the two edges and normalize them
	vector3 edgeRight = verticies[vertRight] - verticies[vertCenter];
	vector3 edgeLeft = verticies[vertLeft] - verticies[vertCenter];
	edgeRight.Normalize();
	edgeLeft.Normalize();

	//Calculate and set the normal
	vector3 normal = edgeRight.Cross(edgeLeft);
	normal.Normalize();
	normals[vertCenter] += normal;

	//Increment the number of normals & tangents calculated
	normalCount[vertCenter]++;

	//Calculate the two texture-coordinate edges
	vector2 texEdgeRight = texcoords[vertRight] - texcoords[vertCenter];
	vector2 texEdgeLeft = texcoords[vertLeft] - texcoords[vertCenter];
	texEdgeRight.Normalize();
	texEdgeLeft.Normalize();

	//Calculate the determinant
	float det = (texEdgeRight.x * texEdgeLeft.y) - (texEdgeRight.y * texEdgeLeft.x);

	vector3 tangent;
	//If the determinant is close enough to 0, set the tangent explicitly
	if (close_enough(det, 0.0f))
	{
		tangent.x = 1.0f;
		tangent.y = 0.0f;
		tangent.z = 0.0f;
	}
	else
	{
		//Calculate the tangent and normalize
		det = 1.0f / det;
		tangent.x = (texEdgeLeft.y * edgeRight.x - texEdgeRight.y * edgeLeft.x) * det;
		tangent.y = (texEdgeLeft.y * edgeRight.y - texEdgeRight.y * edgeLeft.y) * det;
		tangent.z = (texEdgeLeft.y * edgeRight.z - texEdgeRight.y * edgeLeft.z) * det;
		tangent.Normalize();
	}

	//Set the tangent
	tangents[vertCenter] += tangent;
}

//Average out the normals and tangents since they are the combined from neighbouring tris
void VBOData::AverageNormalsTangents(void)
{
	for (int i = 0; i < numOfVerticies; i++)
	{
		normals[i] /= (float)normalCount[i];
		tangents[i] /= (float)normalCount[i];
	}
}

//Bind the data to the VAO & VBO's
void VBOData::BindVBOData(void)
{
	//Create the vertex array
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//Generate three VBOs for vertices, indices, colors
	glGenBuffers(VBOCOUNT, VBO);

	//Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * numOfVerticies, verticies, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//Setup the texcoords buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector2) * numOfVerticies, texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	//Setup the normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * numOfVerticies, normals, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	//Setup the tangent buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * numOfVerticies, tangents, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	//Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numOfIndicies, indicies, GL_STATIC_DRAW);

	//Clear the data from system memory
	DeleteData();
}

//Delete the VBO data from system memory
void VBOData::DeleteData(void)
{
	//Make sure that the data can only be deleted once
	if (!dataDeleted)
	{
		dataDeleted = true;

		RE_DELETE_ARR(verticies);
		RE_DELETE_ARR(texcoords);
		RE_DELETE_ARR(normals);
		RE_DELETE_ARR(tangents);
		RE_DELETE_ARR(normalCount);
		RE_DELETE_ARR(indicies);
	}
}

//--------------------------------------------------------------------------------
//	End Class:	VBOData
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
//	Class:	ShaderManager
//			Used to manage multiple shaders
//--------------------------------------------------------------------------------

//Constructor
ShaderManager::ShaderManager(void)
{
	curIndex = 0;
}

//Destructor
ShaderManager::~ShaderManager(void)
{
	//Delete each shader object
	for (int i = 0; i < curIndex; i++)
		RE_DELETE(shaders[i]);

	//Delete shader array
	RE_DELETE_ARR(*shaders);
}

//Add a new set of shaders
bool ShaderManager::AddShader(string vert, string geom, string frag)
{
	//If out of available shader spots then return an error
	if ((curIndex + 1) > SHADERCOUNT)
		return (false);

	//Create the new shader and increment the current index
	shaders[curIndex++] = new ShaderProg(vert, geom, frag);

	//Return successful
	return (true);
}

//Bind the specific variable to a location in the shaders
void ShaderManager::BindAttrib(char *name, int val)
{
	for (int i = 0; i < curIndex; i++)
		glBindAttribLocation(shaders[i]->m_programID, val, name);
}

//Update a uniform value for an int
void ShaderManager::UpdateUni1i(char *name, int val)
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform1i(glGetUniformLocation(shaders[i]->m_programID, name), val);
	}
}

//Update a uniform value for a float
void ShaderManager::UpdateUni1f(char *name, float val)
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform1f(glGetUniformLocation(shaders[i]->m_programID, name), val);
	}
}

//Update a uniform value for a float
void ShaderManager::UpdateUni2fv(char *name, float val[2])
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform2fv(glGetUniformLocation(shaders[i]->m_programID, name), 1, val);
	}
}

//Update a uniform value for a float3
void ShaderManager::UpdateUni3fv(char *name, float val[3])
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniform3fv(glGetUniformLocation(shaders[i]->m_programID, name), 1, val);
	}
}

//Update a uniform value for a matrix3
void ShaderManager::UpdateUniMat3fv(char *name, float val[9])
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniformMatrix3fv(glGetUniformLocation(shaders[i]->m_programID, name), 1, GL_FALSE, val);
	}
}

//Update a uniform value for a matrix4
void ShaderManager::UpdateUniMat4fv(char *name, float val[16])
{
	for (int i = 0; i < curIndex; i++)
	{
		glUseProgram(shaders[i]->m_programID);
		glUniformMatrix4fv(glGetUniformLocation(shaders[i]->m_programID, name), 1, GL_FALSE, val);

	}
}

//Compile and link the shaders
int ShaderManager::CompileAndLink(void)
{
	for (int i = 0; i < curIndex; i++)
	{
		//Check for errors in compiling any shader
		if (!shaders[i]->CompileAndLink())
			return (0);
	}

	//Success
	return (1);
}

//Set the active shader to be used
void ShaderManager::SetActiveShader(int shader)
{
	if (shader >= SHADERCOUNT)
		return;
	glUseProgram(shaders[shader]->m_programID);
}

//--------------------------------------------------------------------------------
//	End Class:	ShaderManager
//--------------------------------------------------------------------------------
